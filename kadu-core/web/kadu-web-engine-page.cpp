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

#include "kadu-web-engine-page.h"
#include "kadu-web-engine-page.moc"

KaduWebEnginePage::KaduWebEnginePage(QWebEngineProfile *profile, QObject *parent) : QWebEnginePage{profile, parent}
{
}

KaduWebEnginePage::~KaduWebEnginePage()
{
}

bool KaduWebEnginePage::acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame)
{
    if (type == QWebEnginePage::NavigationTypeLinkClicked)
    {
        emit linkClicked(url);
        return false;
    }

    // Everything else is Kadu putting content into the view itself -- setHtml() arrives here as
    // NavigationTypeTyped or NavigationTypeOther and has to be allowed through.
    return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
}
