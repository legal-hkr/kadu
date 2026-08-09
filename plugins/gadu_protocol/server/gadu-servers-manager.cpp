/*
 * %kadu copyright begin%
 * Copyright 2011, 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2011, 2012, 2013 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include <libgadu.h>

#include "gadu-servers-manager.h"

namespace
{
const int MaxDirectAttempts = 3;
const int MaxHubAttempts = 4;
const int DirectAttemptDelay = 5 * 1000;
const int HubAttemptDelay = 15 * 1000;

/**
 * @short The entry standing for the hub rather than for a server.
 *
 * An address of zero is how libgadu is told to go and ask appmsg.gadu-gadu.pl which server is up,
 * instead of being handed one to connect to.
 */
GaduServersManager::GaduServer hub()
{
    return {QHostAddress{(quint32)0}, 0};
}
}

GaduServersManager::GaduServersManager(QObject *parent) : QObject{parent}, DirectAttempts{0}, HubAttempts{0}
{
}

GaduServersManager::~GaduServersManager()
{
}

bool GaduServersManager::isRetryingLastWorkingServer() const
{
    return !LastWorkingServer.first.isNull() && DirectAttempts < MaxDirectAttempts;
}

GaduServersManager::GaduServer GaduServersManager::getServer()
{
    if (!hasAnotherAttempt())
        startOver();

    return isRetryingLastWorkingServer() ? LastWorkingServer : hub();
}

void GaduServersManager::connectionSucceeded(const GaduServersManager::GaduServer &server)
{
    LastWorkingServer = server;
    startOver();
}

void GaduServersManager::attemptFailed()
{
    if (isRetryingLastWorkingServer())
        ++DirectAttempts;
    else
        ++HubAttempts;
}

bool GaduServersManager::hasAnotherAttempt() const
{
    return isRetryingLastWorkingServer() || HubAttempts < MaxHubAttempts;
}

int GaduServersManager::delayBeforeNextAttempt() const
{
    return isRetryingLastWorkingServer() ? DirectAttemptDelay : HubAttemptDelay;
}

void GaduServersManager::startOver()
{
    DirectAttempts = 0;
    HubAttempts = 0;
}
