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

#include <KIdleTime>

#include <QtCore/QDateTime>

namespace
{
/*
 * The Wayland idle protocol announces, it does not answer questions.
 * KIdleTime::idleTime() returns zero under every Wayland compositor -- measured, not assumed --
 * and the only information that arrives is timeoutReached() when idleness begins and
 * resumingFromIdle() when it ends. secondsIdle() is a question, so the moment idleness started is
 * recorded from the announcement and the answer is computed from it.
 *
 * The timeout is short because it only marks the start; how long the caller considers idle enough
 * is the caller's own business.
 */
constexpr int IdleTimeoutMsec = 1000;

QDateTime IdleSince;
int IdleTimeoutId = -1;
}

Idle::Idle(QObject *parent) : QObject(parent)
{
    auto *idleTime = KIdleTime::instance();

    connect(idleTime, &KIdleTime::timeoutReached, this, [idleTime](int identifier, int msec) {
        if (identifier != IdleTimeoutId)
            return;

        // The announcement arrives once idleness has already lasted msec, so that much is subtracted.
        IdleSince = QDateTime::currentDateTime().addMSecs(-msec);

        // Nothing is said about the user returning unless it is asked for.
        idleTime->catchNextResumeEvent();
    });

    connect(idleTime, &KIdleTime::resumingFromIdle, this, [] { IdleSince = QDateTime{}; });

    IdleTimeoutId = idleTime->addIdleTimeout(IdleTimeoutMsec);
}

Idle::~Idle()
{
    if (IdleTimeoutId >= 0)
    {
        KIdleTime::instance()->removeIdleTimeout(IdleTimeoutId);
        IdleTimeoutId = -1;
    }

    IdleSince = QDateTime{};
}

long Idle::secondsIdle()
{
    if (IdleSince.isNull())
        return 0;

    return IdleSince.secsTo(QDateTime::currentDateTime());
}
