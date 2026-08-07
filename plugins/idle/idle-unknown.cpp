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

#include "idle.h"

/*
 * Used when KIdleTime is absent. There is no other way for a Wayland client to learn how long the
 * user has been away, so the honest answer is that it is not known -- which callers must handle
 * anyway, since no compositor is obliged to offer the information.
 */

Idle::Idle(QObject *parent) : QObject(parent)
{
}

Idle::~Idle()
{
}

long Idle::secondsIdle()
{
    return -1;
}
