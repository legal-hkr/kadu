/*
 * %kadu copyright begin%
 * Copyright 2012 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "url-clipboard-html-transformer.h"

// Assume we don't use apostrophes in HTML attributes.

// Expected string to replace is as follows (capitalics are captured):
// <a folded="1" displaystr="DISPLAY" href="HREF"*>DISPLAY</a>
// If first display is different than the second, it means that the user selected only part of the link.
// Source string is created in StandardUrlHandler::convertUrlsToHtml().
// BTW, I know it is totally ugly.
UrlClipboardHtmlTransformer::UrlClipboardHtmlTransformer()
        : UrlRegExp(
              "<a[^>]+folded\\s*=\\s*\"1\"[^>]+displaystr\\s*=\\s*\"([^\"]+)\"[^>]+href\\s*=\\s*\"([^\"]+)\"[^>]*>([^<]"
              "*)<[^>]*>")
{
}

UrlClipboardHtmlTransformer::~UrlClipboardHtmlTransformer()
{
}

QString UrlClipboardHtmlTransformer::transform(const QString &clipboardHtml)
{
    QString result = clipboardHtml;

    int pos = 0;
    while (true)
    {
        auto const match = UrlRegExp.match(result, pos);
        if (!match.hasMatch())
            break;

        pos = match.capturedStart();
        int matchedLength = match.capturedLength();
        QString displayStr = match.captured(1);
        QString realDisplayStr = match.captured(3);

        if (displayStr == realDisplayStr)   // i.e., we are copying the entire link, not a part of it
        {
            QString hRef = match.captured(2);
            QString unfoldedLink = QString("<a href=\"%1\">%1</a>").arg(hRef);
            result.replace(pos, matchedLength, unfoldedLink);

            pos += unfoldedLink.length();
        }
        else
            pos += matchedLength;
    }

    return result;
}
