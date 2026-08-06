/*
 * %kadu copyright begin%
 * Copyright 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2010, 2011 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2011, 2012 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2010, 2011, 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * %kadu copyright end%
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "contact-parser-tags.h"
#include "contact-parser-tags.moc"

#include <QtGui/QGuiApplication>

#include "accounts/account.h"
#include "icons/icons-manager.h"
#include "icons/kadu-icon.h"
#include "misc/paths-provider.h"
#include "parser/parser.h"
#include "protocols/protocol.h"
#include "status/status-container-manager.h"
#include "status/status-type-manager.h"
#include "talkable/talkable-converter.h"

ContactParserTags::ContactParserTags(QObject *parent) : QObject{parent}
{
}

ContactParserTags::~ContactParserTags()
{
}

void ContactParserTags::setIconsManager(IconsManager *iconsManager)
{
    m_iconsManager = iconsManager;
}

void ContactParserTags::setParser(Parser *parser)
{
    m_parser = parser;
}

void ContactParserTags::setStatusContainerManager(StatusContainerManager *statusContainerManager)
{
    m_statusContainerManager = statusContainerManager;
}

void ContactParserTags::setStatusTypeManager(StatusTypeManager *statusTypeManager)
{
    m_statusTypeManager = statusTypeManager;
}

void ContactParserTags::setTalkableConverter(TalkableConverter *talkableConverter)
{
    m_talkableConverter = talkableConverter;
}

namespace
{
/**
 * @short Returns the icon at the resolution the screen has, for a tag placed in rich text.
 *
 * Every reader of these tags -- the tooltip, the information panel syntaxes, the contact grid --
 * sizes the image itself at sixteen units. Qt's rich text engine takes an image's pixel count for
 * that many logical units, so on a magnified screen a sixteen pixel file is enlarged to fill the
 * sixteen units it is given. Handing it a file with as many pixels as the screen will draw leaves
 * the size on screen unchanged and the image sharp.
 */
KaduIcon atScreenResolution(KaduIcon icon)
{
    auto const requested = icon.size().section('x', 0, 0).toInt();
    if (requested <= 0)
        return icon;

    auto const scaled = qRound(requested * qApp->devicePixelRatio());
    icon.setSize(QStringLiteral("%1x%1").arg(scaled));
    return icon;
}
}

void ContactParserTags::init()
{
    m_parser->registerTag("avatarPath", [this](Talkable talkable) {
        auto avatarPath = m_talkableConverter->toAvatarPath(talkable);
        if (avatarPath.isEmpty())
            return QString{};
        else
            return PathsProvider::webKitPath(avatarPath);
    });
    m_parser->registerTag("statusIconPath", [this](Talkable talkable) {
        if (m_talkableConverter->toBuddy(talkable).isBlocked())
            return PathsProvider::webKitPath(m_iconsManager->iconPath(atScreenResolution(KaduIcon{"kadu_icons/blocked", "16x16"})));

        if (m_talkableConverter->toContact(talkable).isBlocking())
            return PathsProvider::webKitPath(m_iconsManager->iconPath(atScreenResolution(KaduIcon{"kadu_icons/blocking", "16x16"})));

        auto status = m_talkableConverter->toStatus(talkable);
        auto account = m_talkableConverter->toAccount(talkable);
        if (auto protocol = account.protocolHandler())
            return PathsProvider::webKitPath(
                m_iconsManager->iconPath(atScreenResolution(m_statusTypeManager->statusIcon(protocol->statusPixmapPath(), status))));
        else
            return PathsProvider::webKitPath(
                m_iconsManager->iconPath(m_statusContainerManager->statusIcon(Status{status.type()})));
    });
}

void ContactParserTags::done()
{
    m_parser->unregisterTag("avatarPath");
    m_parser->unregisterTag("statusIconPath");
}
