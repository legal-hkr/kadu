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

#include <QtCore/QObject>

/**
 * @short Tells the rest of the program when the desktop changes its colours.
 *
 * A desktop that turns dark at dusk hands its programs a new palette while they are running. The
 * colours Kadu draws with are worked out in configurationUpdated(), which until now was only ever
 * reached by the user changing a setting -- so a window that was light at noon stayed light at
 * midnight. This watches the application for the palette it is given and asks everyone to work
 * their colours out again, which is the same path a settings change takes.
 */
class KADUAPI SystemColorsWatcher : public QObject
{
    Q_OBJECT

public:
    explicit SystemColorsWatcher(QObject *parent = nullptr);
    virtual ~SystemColorsWatcher();

protected:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void reapplyStyleSheet();
};
