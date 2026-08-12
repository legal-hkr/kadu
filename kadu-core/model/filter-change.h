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

#include <QtCore/QtGlobal>

/**
 * Announcing a change to what a proxy model lets through.
 *
 * Qt 6.10 split the old single call, made once the criteria had already been changed, into a pair
 * surrounding the change: the model looks at what it accepts before and after and can then report
 * the rows that came and went, rather than declaring everything it holds to be new. The old call is
 * deprecated as of Qt 6.13.
 *
 * These stand in for the pair, so that the criteria can be changed the same way whichever Qt the
 * port is built against. Written as macros rather than a class holding the model, because the three
 * functions are protected: only the model itself may call them, and a macro is expanded inside it.
 *
 * Between the two, change the criteria and nothing else -- the model reads its own state twice, and
 * anything else done in there is done while it is halfway through looking.
 */
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
#define KADU_BEGIN_FILTER_CHANGE() beginFilterChange()
#define KADU_END_FILTER_CHANGE_ROWS() endFilterChange(Direction::Rows)
#define KADU_END_FILTER_CHANGE_BOTH() endFilterChange(Direction::Both)
#else
#define KADU_BEGIN_FILTER_CHANGE() ((void)0)
#define KADU_END_FILTER_CHANGE_ROWS() invalidateFilter()
#define KADU_END_FILTER_CHANGE_BOTH() invalidateFilter()
#endif
