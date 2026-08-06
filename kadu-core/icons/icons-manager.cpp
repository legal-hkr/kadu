/*
 * %kadu copyright begin%
 * Copyright 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2011, 2012, 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2011, 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "icons-manager.h"
#include "icons-manager.moc"

#include "accounts/account-manager.h"
#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "core/core.h"
#include "icons/kadu-icon.h"
#include "misc/misc.h"
#include "protocols/protocol.h"
#include "themes/icon-theme-manager.h"

#include <QtCore/QRegularExpression>
#include <QtCore/QFileInfo>

namespace
{
/**
 * @short Icon sizes a theme may provide, smallest first.
 *
 * 48x48 used to be missing from this list although the bundled themes ship 45 icons in it, so
 * those files were never offered to anyone.
 */
const QStringList &iconSizes()
{
    static const QStringList sizes{QStringLiteral("16x16"), QStringLiteral("22x22"), QStringLiteral("32x32"),
                                   QStringLiteral("48x48"), QStringLiteral("64x64"), QStringLiteral("96x96"),
                                   QStringLiteral("128x128"), QStringLiteral("256x256")};
    return sizes;
}
}

IconsManager::IconsManager(QObject *parent) : QObject{parent}, UseSystemIcons(true)
{
}

IconsManager::~IconsManager()
{
}

void IconsManager::setAccountManager(AccountManager *accountManager)
{
    m_accountManager = accountManager;
}

void IconsManager::setConfiguration(Configuration *configuration)
{
    m_configuration = configuration;
}

void IconsManager::setIconThemeManager(IconThemeManager *iconThemeManager)
{
    m_iconThemeManager = iconThemeManager;
}

void IconsManager::init()
{
    m_iconThemeManager->loadThemes();
    configurationUpdated();

    // TODO: localized protocol
    localProtocolPath = "gadu-gadu";
}

QString IconsManager::iconPath(
    const KaduIcon &icon, IconsManager::AllowEmpty allowEmpty, IconsManager::SizeMatch sizeMatch) const
{
    QString path = icon.path();
    QString size = icon.size();

    QFileInfo fileInfo(path);
    QString themePath = icon.themePath().isEmpty() ? m_iconThemeManager->currentTheme().path() : icon.themePath();
    QString name = fileInfo.fileName();
    QString realPath = fileInfo.path();

    auto fileForSize = [&themePath, &realPath, &name](const QString &wantedSize) {
        QFileInfo candidate{themePath + realPath + '/' + wantedSize + '/' + name + ".png"};
        if (candidate.isFile() && candidate.isReadable())
            return candidate.canonicalFilePath();

        candidate.setFile(themePath + realPath + '/' + wantedSize + '/' + name + ".gif");
        if (candidate.isFile() && candidate.isReadable())
            return candidate.canonicalFilePath();

        return QString{};
    };

    auto found = fileForSize(size);
    if (!found.isEmpty())
        return found;

    // A theme carries neither every icon in every size nor an entry for callers that ask for no
    // size at all: protocols/xmpp/xmpp exists only at 32x32 while the protocol asks for 16x16, and
    // the Gadu-Gadu factory names no size whatsoever. Dropping through to the placeholder threw
    // those icons away, so take the nearest size the theme does have.
    if (AnySize == sizeMatch)
    {
        auto const requested = size.section('x', 0, 0).toInt();
        auto sizes = iconSizes();
        std::sort(sizes.begin(), sizes.end(), [requested](const QString &left, const QString &right) {
            return qAbs(left.section('x', 0, 0).toInt() - requested) <
                   qAbs(right.section('x', 0, 0).toInt() - requested);
        });

        for (auto const &candidateSize : std::as_const(sizes))
        {
            found = fileForSize(candidateSize);
            if (!found.isEmpty())
                return found;
        }
    }

    if (realPath == QStringLiteral("protocols/common"))
    {
        QString protocolPath;
        if (m_accountManager->defaultAccount().protocolHandler())
            protocolPath = m_accountManager->defaultAccount().protocolHandler()->statusPixmapPath();
        else
            protocolPath = localProtocolPath;

        KaduIcon protocolPathIcon = icon;
        protocolPathIcon.setPath(QString("protocols/%1/%2").arg(protocolPath).arg(name));
        return iconPath(protocolPathIcon, allowEmpty, sizeMatch);
    }

    if (EmptyAllowed == allowEmpty)
        return QString();
    else
        return iconPath(KaduIcon("kadu_icons/0", size), EmptyAllowed, sizeMatch);
}

QIcon IconsManager::buildPngIcon(const QString &themePath, const QString &path)
{
    QIcon icon;
    for (auto const &size : iconSizes())
    {
        KaduIcon kaduIcon(path, size);
        kaduIcon.setThemePath(themePath);

        // Only an exact match belongs in a multi-size icon: letting iconPath() substitute a
        // different size would add the same file several times over.
        QString fullPath = iconPath(kaduIcon, EmptyAllowed, ExactSizeOnly);
        if (!fullPath.isEmpty())
            icon.addFile(fullPath);
    }

    return icon;
}

QIcon IconsManager::iconByPath(const QString &themePath, const QString &path, AllowEmpty allowEmpty)
{
    if (!IconCache.contains(themePath + path))
    {
        QIcon icon;

        QFileInfo fileInfo(path);
        if (fileInfo.isAbsolute() && fileInfo.isReadable())
            icon.addFile(path);
        else
        {
            // Icons named after the freedesktop standard -- the ones with no directory in their
            // path, such as application-exit or document-open -- are what the desktop's own theme
            // provides, in every size and usually as vectors. Half of the bundled ones exist at
            // 16x16 and nothing else, which is a poor showing on a magnified screen, so ask the
            // desktop first and keep the bundled files as the answer when it has nothing.
            if (UseSystemIcons && !path.contains('/'))
                icon = QIcon::fromTheme(path);

            if (icon.isNull())
                icon = buildPngIcon(themePath, path);

            if (icon.isNull())
            {
                static const QRegularExpression commonRegexp{QStringLiteral("^protocols/common/(.+)$")};
                auto const commonMatch = commonRegexp.match(path);
                if (commonMatch.hasMatch())
                {
                    QString protocolpath;
                    if (m_accountManager->defaultAccount().protocolHandler())
                        protocolpath = m_accountManager->defaultAccount().protocolHandler()->statusPixmapPath();
                    else
                        protocolpath = localProtocolPath;
                    return iconByPath(themePath, QString("protocols/%1/%2").arg(protocolpath, commonMatch.captured(1)));
                }
            }

            if (icon.isNull() && EmptyNotAllowed == allowEmpty)
                icon = buildPngIcon(themePath, "kadu_icons/0");
        }

        IconCache.insert(themePath + path, icon);
    }

    return IconCache.value(themePath + path);
}

QIcon IconsManager::iconByPath(const KaduIcon &icon)
{
    return iconByPath(icon.themePath(), icon.path());
}

void IconsManager::clearCache()
{
    IconCache.clear();
}

void IconsManager::configurationUpdated()
{
    bool const useSystemIcons = m_configuration->deprecatedApi()->readBoolEntry("Look", "UseSystemIcons", true);
    if (useSystemIcons != UseSystemIcons)
    {
        UseSystemIcons = useSystemIcons;
        clearCache();

        emit themeChanged();
    }

    bool themeWasChanged =
        m_configuration->deprecatedApi()->readEntry("Look", "IconTheme") != m_iconThemeManager->currentTheme().name();
    if (themeWasChanged)
    {
        clearCache();
        m_iconThemeManager->setCurrentTheme(m_configuration->deprecatedApi()->readEntry("Look", "IconTheme"));
        m_configuration->deprecatedApi()->writeEntry("Look", "IconTheme", m_iconThemeManager->currentTheme().name());

        emit themeChanged();
    }
}

QSize IconsManager::getIconsSize()
{
    return QSize(16, 16);
}
