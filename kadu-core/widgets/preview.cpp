/*
 * %kadu copyright begin%
 * Copyright 2009, 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2011 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2009 Bartłomiej Zimoń (uzi18@o2.pl)
 * Copyright 2010, 2011, 2012, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2009, 2010, 2011, 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "preview.h"
#include "preview.moc"

#include "buddies/buddy-dummy-factory.h"
#include "parser/parser.h"
#include "widgets/kadu-web-view.h"

#include <QtWidgets/QHBoxLayout>

#include "core/injected-factory.h"

#define PREVIEW_DEFAULT_HEIGHT 250

Preview::Preview(QWidget *parent) : QFrame(parent), m_webView{nullptr}, m_layout{nullptr}
{
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setFixedHeight(PREVIEW_DEFAULT_HEIGHT);

    // Expanding rather than Preferred: QWebEngineView derives its size hint from the loaded
    // contents and reports 0x0 until something is rendered, which left this frame two pixels wide.
    // QWebView used to answer with the page's preferred size instead, so Preferred was enough.
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_layout = layout;
}

void Preview::setInjectedFactory(InjectedFactory *injectedFactory)
{
    m_injectedFactory = injectedFactory;
}

void Preview::init()
{
    // The view has to come from the injected factory: KaduWebView is given the shared QtWebEngine
    // profile through injection, and a plain new would leave it without one.
    m_webView = m_injectedFactory->makeInjected<KaduWebView>(this);
    m_webView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_layout->addWidget(m_webView);

    // QWebEnginePage has no palette; the transparent page background replaces QPalette::Base.
    m_webView->page()->setBackgroundColor(Qt::transparent);
    m_webView->setAttribute(Qt::WA_OpaquePaintEvent, false);
}

Preview::~Preview()
{
}

void Preview::setBuddyDummyFactory(BuddyDummyFactory *buddyDummyFactory)
{
    m_buddyDummyFactory = buddyDummyFactory;
}

void Preview::setParser(Parser *parser)
{
    m_parser = parser;
}

KaduWebView *Preview::webView() const
{
    return m_webView;
}

void Preview::syntaxChanged(const QString &content)
{
    QString syntax = content;
    QString text = m_parser->parse(syntax, Talkable(m_buddyDummyFactory->dummy()), ParserEscape::HtmlEscape);
    emit needFixup(text);

    m_webView->setHtml(text);
}
