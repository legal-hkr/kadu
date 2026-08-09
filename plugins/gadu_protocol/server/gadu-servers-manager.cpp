/*
 * %kadu copyright begin%
 * Copyright 2011, 2012 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2011, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

GaduServersManager::GaduServersManager(QObject *parent) : QObject{parent}
{
}

GaduServersManager::~GaduServersManager()
{
}

namespace
{
/**
 * @short Whether this entry stands for the hub rather than for a server.
 *
 * An address of zero is how libgadu is told to go and ask the hub which server is up, instead of
 * being handed one to connect to.
 */
bool isHub(const GaduServersManager::GaduServer &server)
{
    return server.first.isNull() || 0 == server.first.toIPv4Address();
}
}

void GaduServersManager::init()
{
    // The hub, and nothing besides. Sixteen addresses used to stand here too, 91.214.237.108 up to
    // 123, and not one of them answers any more -- every attempt on them runs out its clock, and
    // the clock is capped at fifteen seconds each. Asking all sixteen before the hub is asked again
    // is what a connection cut while the machine slept looked like from the outside: an account
    // that blinks and never comes back.
    //
    // The hub is asked afresh every time, which is what the first connection after starting does --
    // and that one has always worked.
    GoodServers << GaduServer{QHostAddress{(quint32)0}, 0};
}

GaduServersManager::GaduServer GaduServersManager::getServer()
{
    if (GoodServers.isEmpty())
    {
        GoodServers = BadServers;
        BadServers.clear();
        return GaduServer(QHostAddress(), 0);
    }

    if (GoodServers[0].second != 443 && GoodServers[0].second != 0)
    {
        markServerAsBad(GoodServers[0]);
        return getServer();
    }

    return GoodServers[0];
}

void GaduServersManager::markServerAsBad(GaduServersManager::GaduServer server)
{
    // The hub cannot be a bad server because it is not a server: it is the thing that says which
    // server to use. It was being thrown out on the first failure all the same, including failures
    // that say nothing whatever about it -- a connection cut while the machine slept is not the
    // hub's doing -- and everything after that went to the wrong addresses.
    if (isHub(server))
        return;

    GoodServers.removeAll(server);
    BadServers.append(server);
}
