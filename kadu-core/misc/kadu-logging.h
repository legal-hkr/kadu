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

#include "exports.h"

#include <QtCore/QLoggingCategory>

/**
 * @short Logging categories for the paths that decide whether an account connects.
 *
 * Both default to QtWarningMsg, so a normal run says nothing. Enable them when an account
 * stops connecting without reporting anything:
 *
 *     QT_LOGGING_RULES="kadu.protocol.state.debug=true;kadu.status.change.debug=true" kadu
 *
 * kadu.protocol.state announces every state machine transition. kadu.status.change covers the
 * steps before it: an account only leaves the logged-out-online state when a status is pushed
 * down to its protocol, and there are several places where that push is dropped silently.
 */
KADUAPI Q_DECLARE_LOGGING_CATEGORY(KADU_PROTOCOL_STATE)
KADUAPI Q_DECLARE_LOGGING_CATEGORY(KADU_STATUS_CHANGE)
