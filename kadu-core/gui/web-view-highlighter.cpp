/*
 * %kadu copyright begin%
 * Copyright 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include <QtCore/QPointer>
#include <QtWebEngineCore/QWebEngineFindTextResult>
#include <QtWebEngineCore/QWebEnginePage>

#include "widgets/webkit-messages-view/webkit-messages-view.h"

#include "web-view-highlighter.h"
#include "web-view-highlighter.moc"

WebViewHighlighter::WebViewHighlighter(WebkitMessagesView *parent) : QObject(parent), AutoUpdate(false)
{
}

WebViewHighlighter::~WebViewHighlighter()
{
}

WebkitMessagesView *WebViewHighlighter::chatMessagesView() const
{
    return static_cast<WebkitMessagesView *>(parent());
}

void WebViewHighlighter::setAutoUpdate(const bool autoUpdate)
{
    if (AutoUpdate == autoUpdate)
        return;

    // we might assume that when any message is added to web view its size changes
    // unfortunately contentChanged() does not work when content is updated by javascript

    if (AutoUpdate)
        disconnect(
            chatMessagesView()->page(), &QWebEnginePage::contentsSizeChanged, this,
            &WebViewHighlighter::updateHighlighting);

    AutoUpdate = autoUpdate;

    if (AutoUpdate)
        connect(
            chatMessagesView()->page(), &QWebEnginePage::contentsSizeChanged, this,
            &WebViewHighlighter::updateHighlighting);
}

void WebViewHighlighter::setHighlight(const QString &highlightString)
{
    if (HighlightString == highlightString)
        return;

    clearHighlighting();
    HighlightString = highlightString;
    updateHighlighting();
}

void WebViewHighlighter::find(const QString &text, QWebEnginePage::FindFlags flags, bool updateAtBottom)
{
    // QtWebKit's findText() answered immediately; QtWebEngine runs the search in the render process
    // and calls back. The result therefore has to be reported from the callback, and the highlighter
    // may be gone by then.
    QPointer<WebViewHighlighter> self{this};
    chatMessagesView()->page()->findText(text, flags, [self, updateAtBottom](const QWebEngineFindTextResult &result) {
        if (!self)
            return;
        if (updateAtBottom)
            self->chatMessagesView()->updateAtBottom();
        emit self->somethingFound(result.numberOfMatches() > 0);
    });
}

void WebViewHighlighter::updateHighlighting()
{
    if (HighlightString.isEmpty())
        return;

    // Repeating a search advances to the next match, so the session is cleared first to make this
    // start from the top again. QtWebEngine highlights every match and wraps on its own, which is
    // what QWebPage::HighlightAllOccurrences and FindWrapsAroundDocument used to ask for.
    chatMessagesView()->page()->findText(QString());
    find(HighlightString, QWebEnginePage::FindFlags{}, false);
}

void WebViewHighlighter::clearHighlighting()
{
    chatMessagesView()->page()->findText(QString());

    emit somethingFound(true);
}

void WebViewHighlighter::selectNext(const QString &select)
{
    find(select, QWebEnginePage::FindFlags{}, true);
}

void WebViewHighlighter::selectPrevious(const QString &select)
{
    find(select, QWebEnginePage::FindBackward, true);
}

void WebViewHighlighter::clearSelect()
{
    chatMessagesView()->page()->findText(QString());
    chatMessagesView()->updateAtBottom();

    emit somethingFound(true);

    updateHighlighting();
}
