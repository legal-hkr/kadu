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

#include "configuration/configuration-aware-object.h"

#include "system-colors-watcher.h"
#include "system-colors-watcher.moc"

#include <QtCore/QCoreApplication>
#include <QtCore/QEvent>
#include <QtCore/QTimer>
#include <QtWidgets/QApplication>

SystemColorsWatcher::SystemColorsWatcher(QObject *parent) : QObject{parent}, m_pending{false}
{
    if (auto *application = QCoreApplication::instance())
        application->installEventFilter(this);
}

SystemColorsWatcher::~SystemColorsWatcher()
{
}

void SystemColorsWatcher::reapplyStyleSheet()
{
    auto *application = qobject_cast<QApplication *>(QCoreApplication::instance());
    if (!application)
        return;

    // An application carrying a style sheet has every widget drawn through the style sheet style,
    // and that one works its colours out when the sheet is set and then keeps them: measured on a
    // menu bar and a tool bar, their palettes sat at the colours of the first palette they ever
    // saw and did not move again, however many times the desktop changed. Setting the same sheet
    // again is what makes them look afresh.
    auto const styleSheet = application->styleSheet();
    if (styleSheet.isEmpty())
        return;

    application->setStyleSheet(QString{});
    application->setStyleSheet(styleSheet);
}

void SystemColorsWatcher::colorsChanged()
{
    m_pending = false;

    reapplyStyleSheet();
    ConfigurationAwareObject::notifyAll();
}

bool SystemColorsWatcher::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::ApplicationPaletteChange && !m_pending)
    {
        // Answered on the next turn of the event loop rather than here. A desktop changing its
        // colours does not hand the new palette out in one go, and the announcement arrives while
        // that is still under way -- reading the colours at this point gave a window whose text
        // had moved on while its background had not. Waiting until the turn is over reads one
        // palette rather than half of each.
        //
        // The announcement also arrives many times over, once for each widget that is told; the
        // flag turns that crowd into the single piece of news it really is.
        m_pending = true;
        QTimer::singleShot(0, this, &SystemColorsWatcher::colorsChanged);
    }

    return QObject::eventFilter(watched, event);
}
