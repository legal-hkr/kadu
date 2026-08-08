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

#include "kadu-config.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QEvent>
#include <QtCore/QTimer>

#if HAVE_KCONFIG
#include <KSharedConfig>
#endif

SystemColorsWatcher::SystemColorsWatcher(QObject *parent) : QObject{parent}, m_pending{false}
{
    if (auto *application = QCoreApplication::instance())
        application->installEventFilter(this);
}

SystemColorsWatcher::~SystemColorsWatcher()
{
}

void SystemColorsWatcher::colorsChanged()
{
    m_pending = false;

    ConfigurationAwareObject::notifyAll();
}

void SystemColorsWatcher::rereadDesktopConfiguration()
{
#if HAVE_KCONFIG
    // Read here rather than a turn later, and before returning: the style is listening for this
    // same announcement, and filters are called in the reverse of the order they were installed --
    // this one goes on last and so is heard first. The style then works its colours out from the
    // configuration as it now stands.
    //
    // Without this the menu bar and the tool bars keep the colours the desktop had when Kadu
    // started. Breeze gives those two a palette of their own, taken from the colours a scheme
    // names for a window's header, and reads them through the configuration file named after the
    // application -- which does not notice kdeglobals being rewritten underneath it. Measured on a
    // bare Qt window: with no application name it follows the desktop, and adding one line,
    // setApplicationName(), is enough to freeze it. Reported to the Breeze authors; until it is
    // fixed there, every Qt6 program with a name and a menu bar needs this.
    KSharedConfig::openConfig()->reparseConfiguration();
#endif
}

bool SystemColorsWatcher::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::ApplicationPaletteChange)
        rereadDesktopConfiguration();

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
