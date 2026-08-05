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

/*
 * Qt6 removed the Qt5X11Extras module and with it QX11Info. The X11 Display
 * is now reached through a native interface, which is only available when the
 * application actually runs on the X11 platform plugin -- under Wayland the
 * interface is absent and the accessor returns nullptr.
 *
 * Callers that used QX11Info::display() unconditionally must therefore be
 * prepared for a null Display; on Wayland the X11-specific window management
 * helpers simply do nothing.
 */

#include <QtGui/QGuiApplication>

using Display = struct _XDisplay;

inline Display *kaduX11Display()
{
#if QT_CONFIG(xcb)
    if (auto x11Application = qApp->nativeInterface<QNativeInterface::QX11Application>())
        return x11Application->display();
#endif
    return nullptr;
}
