/*
 * %kadu copyright begin%
 * Copyright 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "widgets/chat-widget/chat-widget-title-composing-state-position.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QPalette>

#include "chat-configuration-holder.h"
#include "chat-configuration-holder.moc"

ChatConfigurationHolder::ChatConfigurationHolder(QObject *parent)
        : ConfigurationHolder{parent}, AutoSend{}, NiceDateFormat{}, ChatTextCustomColors{}, ForceCustomChatFont{},
          ChatBgFilled{}, UseTransparency{}, ContactStateChats{}, ContactStateWindowTitle{},
          ContactStateWindowTitlePosition{}
{
}

ChatConfigurationHolder::~ChatConfigurationHolder()
{
}

void ChatConfigurationHolder::setConfiguration(Configuration *configuration)
{
    m_configuration = configuration;
}

void ChatConfigurationHolder::init()
{
    configurationUpdated();
}

void ChatConfigurationHolder::configurationUpdated()
{
    AutoSend = m_configuration->deprecatedApi()->readBoolEntry("Chat", "AutoSend");
    NiceDateFormat = m_configuration->deprecatedApi()->readBoolEntry("Look", "NiceDateFormat");

    // Unless the user asks for colours of their own, every colour below is taken from the palette
    // the desktop hands out, so the conversation turns dark when the desktop does. The stored
    // values are read but not used, and are waiting unchanged for the switch to be turned back on.
    auto const customColors = m_configuration->deprecatedApi()->readBoolEntry("Look", "ChatCustomColors");
    auto const palette = QGuiApplication::palette();

    ChatTextCustomColors =
        customColors && m_configuration->deprecatedApi()->readBoolEntry("Look", "ChatTextCustomColors");
    ChatTextBgColor = customColors ? m_configuration->deprecatedApi()->readColorEntry("Look", "ChatTextBgColor")
                                   : palette.base().color();
    ChatTextFontColor = customColors ? m_configuration->deprecatedApi()->readColorEntry("Look", "ChatTextFontColor")
                                     : palette.text().color();

    ForceCustomChatFont = m_configuration->deprecatedApi()->readBoolEntry("Look", "ForceCustomChatFont");
    ChatFont = m_configuration->deprecatedApi()->readFontEntry("Look", "ChatFont");

    // One side of the conversation stands on the window's own background and the other on the
    // colour a list uses for every second row, which is how a desktop tells two kinds of thing
    // apart without naming a colour of its own.
    MyBackgroundColor = customColors ? m_configuration->deprecatedApi()->readEntry("Look", "ChatMyBgColor")
                                     : palette.base().color().name();
    MyFontColor = customColors ? m_configuration->deprecatedApi()->readEntry("Look", "ChatMyFontColor")
                               : palette.text().color().name();
    MyNickColor = customColors ? m_configuration->deprecatedApi()->readEntry("Look", "ChatMyNickColor")
                               : palette.text().color().name();
    UsrBackgroundColor = customColors ? m_configuration->deprecatedApi()->readEntry("Look", "ChatUsrBgColor")
                                      : palette.alternateBase().color().name();
    UsrFontColor = customColors ? m_configuration->deprecatedApi()->readEntry("Look", "ChatUsrFontColor")
                                : palette.text().color().name();
    UsrNickColor = customColors ? m_configuration->deprecatedApi()->readEntry("Look", "ChatUsrNickColor")
                                : palette.text().color().name();

    ContactStateChats = m_configuration->deprecatedApi()->readBoolEntry("Chat", "ContactStateChats");
    ContactStateWindowTitle = m_configuration->deprecatedApi()->readBoolEntry("Chat", "ContactStateWindowTitle");
    ContactStateWindowTitlePosition =
        m_configuration->deprecatedApi()->readNumEntry("Chat", "ContactStateWindowTitlePosition");

    ChatBgFilled = customColors && m_configuration->deprecatedApi()->readBoolEntry("Look", "ChatBgFilled");
    ChatBgColor = customColors ? m_configuration->deprecatedApi()->readColorEntry("Look", "ChatBgColor")
                               : palette.base().color();

    UseTransparency = m_configuration->deprecatedApi()->readBoolEntry("Chat", "UseTransparency");

    emit chatConfigurationUpdated();
}

ChatWidgetTitleComposingStatePosition ChatConfigurationHolder::composingStatePosition() const
{
    if (!ContactStateWindowTitle)
        return ChatWidgetTitleComposingStatePosition::None;
    if (ContactStateWindowTitlePosition == 0)
        return ChatWidgetTitleComposingStatePosition::AtBegining;
    return ChatWidgetTitleComposingStatePosition::AtEnd;
}
