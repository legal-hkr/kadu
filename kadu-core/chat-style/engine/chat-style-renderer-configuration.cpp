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

#include "chat-style-renderer-configuration.h"

ChatStyleRendererConfiguration::ChatStyleRendererConfiguration(
    Chat chat, QWebEnginePage &page, QString javaScript, bool useTransparency)
        : m_chat{std::move(chat)},
          m_page(page), m_javaScript{std::move(javaScript)}, m_useTransparency{useTransparency}
{
}

Chat ChatStyleRendererConfiguration::chat() const
{
    return m_chat;
}

QWebEnginePage &ChatStyleRendererConfiguration::page() const
{
    return m_page;
}

QString ChatStyleRendererConfiguration::javaScript() const
{
    return m_javaScript;
}

bool ChatStyleRendererConfiguration::useTransparency() const
{
    return m_useTransparency;
}

bool operator==(const ChatStyleRendererConfiguration &left, const ChatStyleRendererConfiguration &right)
{
    if (left.useTransparency() != right.useTransparency())
        return false;
    if (std::addressof(left.page()) != std::addressof(right.page()))
        return false;
    if (left.chat() != right.chat())
        return false;
    if (left.javaScript() != right.javaScript())
        return false;
    return true;
}

bool operator!=(const ChatStyleRendererConfiguration &left, const ChatStyleRendererConfiguration &right)
{
    return !(left == right);
}
