/*
 * %kadu copyright begin%
 * Copyright 2026 Kadu Qt6 port
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

#include <QtWebEngineCore/QWebEngineProfile>
#include <QtWebEngineCore/QWebEngineSettings>

#include "kadu-web-engine-profile.h"
#include "kadu-web-engine-profile.moc"

KaduWebEngineProfile::KaduWebEngineProfile(QObject *parent)
        : QObject{parent}, m_profile{std::make_unique<QWebEngineProfile>()}
{
    // Constructing without a storage name gives an off-the-record profile; these two make the
    // intent explicit rather than relying on that default.
    m_profile->setHttpCacheType(QWebEngineProfile::NoCache);
    m_profile->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);

    auto settings = m_profile->settings();

    // Chat styles are driven by JavaScript: the Adium styles append messages through
    // appendMessage(), and every bundled .syntax style updates delivery receipts and the typing
    // indicator from script.
    settings->setAttribute(QWebEngineSettings::JavascriptEnabled, true);

    // Styles reference their CSS, images and scripts by absolute file:// URLs. Without this the
    // engine refuses to load them and the style silently renders unstyled.
    settings->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);

    // ...but local chat content has no business reaching the network. A message is attacker
    // controlled text, so keep it away from remote origins.
    settings->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, false);
    settings->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, false);
    settings->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, false);
    settings->setAttribute(QWebEngineSettings::JavascriptCanPaste, false);
    settings->setAttribute(QWebEngineSettings::HyperlinkAuditingEnabled, false);
    settings->setAttribute(QWebEngineSettings::DnsPrefetchEnabled, false);
    settings->setAttribute(QWebEngineSettings::WebGLEnabled, false);
    settings->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, false);
    settings->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, false);
    settings->setAttribute(QWebEngineSettings::PdfViewerEnabled, false);

    // A chat view is not a document being browsed: a navigation must never pull focus away from
    // the message input, and a failed load must not replace the conversation with a Chromium
    // error page.
    settings->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, false);
    settings->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);

    // Nothing in the view is a form control, so links in the focus chain only add tab stops
    // between the roster and the input box.
    settings->setAttribute(QWebEngineSettings::LinksIncludedInFocusChain, false);
}

KaduWebEngineProfile::~KaduWebEngineProfile()
{
}

QWebEngineProfile *KaduWebEngineProfile::profile() const
{
    return m_profile.get();
}
