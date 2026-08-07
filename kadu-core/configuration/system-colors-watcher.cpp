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

SystemColorsWatcher::SystemColorsWatcher(QObject *parent) : QObject{parent}
{
    if (auto *application = QCoreApplication::instance())
        application->installEventFilter(this);
}

SystemColorsWatcher::~SystemColorsWatcher()
{
}

bool SystemColorsWatcher::eventFilter(QObject *watched, QEvent *event)
{
    // ApplicationPaletteChange is the whole desktop's doing and arrives once; PaletteChange reaches
    // every widget in turn, which would mean recomputing everything as many times as there are
    // windows. Only the first is answered.
    if (event->type() == QEvent::ApplicationPaletteChange)
        ConfigurationAwareObject::notifyAll();

    return QObject::eventFilter(watched, event);
}
