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

#pragma once

#include <QtWebEngineCore/QWebEnginePage>

#include "exports.h"

class QWebEngineProfile;

/**
 * @addtogroup Web
 * @{
 */

/**
 * @class KaduWebEnginePage
 * @short Page that hands every clicked link to Kadu instead of navigating to it.
 *
 * QtWebKit had QWebPage::setLinkDelegationPolicy(DelegateAllLinks), which suppressed navigation and
 * emitted linkClicked() instead. QtWebEngine dropped it, so the same behaviour is obtained by
 * refusing link navigations in acceptNavigationRequest() and emitting linkClicked() from there.
 *
 * This matters beyond convenience: a chat view displays text written by other people. Letting it
 * navigate would turn any received link into an unattended page load in the conversation window.
 * Kadu instead routes the URL through UrlHandlerManager, which opens it in the user's browser.
 */
class KADUAPI KaduWebEnginePage : public QWebEnginePage
{
    Q_OBJECT

public:
    explicit KaduWebEnginePage(QWebEngineProfile *profile, QObject *parent = nullptr);
    virtual ~KaduWebEnginePage();

signals:
    /**
     * @short Emitted instead of navigating when the user clicks a link.
     * @param url address of the clicked link
     */
    void linkClicked(const QUrl &url);

protected:
    virtual bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame) override;
};

/**
 * @}
 */
