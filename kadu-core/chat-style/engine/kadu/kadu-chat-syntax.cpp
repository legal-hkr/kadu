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

#include "kadu-chat-syntax.h"

#include <QtCore/QRegularExpression>

KaduChatSyntax::KaduChatSyntax(const QString &syntax)
{
    setSyntax(syntax);
}

void KaduChatSyntax::setSyntax(const QString &syntax)
{
    Syntax = syntax;
    QString syntax2 = syntax;
    // DotMatchesEverythingOption is not cosmetic here: a <kadu:top> section holds the style's
    // JavaScript and spans many lines. QRegExp's dot covered newlines, PCRE2's does not, so
    // without it every bundled .syntax style would quietly lose its scripts -- and with them the
    // delivery receipts and the typing indicator -- while still loading successfully.
    static const QRegularExpression topRegexp{
        QStringLiteral("<kadu:top>(.*)</kadu:top>"), QRegularExpression::DotMatchesEverythingOption};
    auto const topMatch = topRegexp.match(syntax2);
    if (topMatch.hasMatch())
    {
        Top = topMatch.captured(1);
        syntax2.remove(topMatch.capturedStart(), topMatch.capturedLength());
    }
    else
        Top = QString();
    WithHeader = syntax2;
    WithHeader.remove("<kadu:header>");
    WithHeader.remove("</kadu:header>");
    WithoutHeader = syntax2;
    // Same reason as above: a header section spans lines.
    static const QRegularExpression headerRegexp{
        QStringLiteral("<kadu:header>.*</kadu:header>"), QRegularExpression::DotMatchesEverythingOption};
    WithoutHeader.remove(headerRegexp);
}
