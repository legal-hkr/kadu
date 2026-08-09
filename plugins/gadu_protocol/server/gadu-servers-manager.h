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

#ifndef GADU_SERVERS_MANAGER_H
#define GADU_SERVERS_MANAGER_H

#include <QtCore/QPair>
#include <QtNetwork/QHostAddress>
#include <injeqt/injeqt.h>

#include "../gadu-exports.h"

/**
 * @class GaduServersManager
 * @short Where to connect next, and how long to wait first.
 *
 * A connection that goes away wants picking up again, but not at any price and not for ever. This
 * decides both parts of that: which address the next attempt should use, and how long to leave
 * between one attempt and the next.
 *
 * What it does, in order:
 *
 * Three attempts at the server the account was last connected to, five seconds apart. That address
 * is known to have worked, and a connection cut by a sleeping machine or a changed network usually
 * comes straight back on it, without the round trip through the hub.
 *
 * Then four attempts at the whole procedure, starting from the hub -- appmsg.gadu-gadu.pl, which
 * says which server is up -- fifteen seconds apart. This is what happens if the remembered server
 * is not the one to use any more, and it is also where an account with nothing remembered begins.
 *
 * Then nothing. The attempts are not repeated until somebody asks for a connection again, at which
 * point the counting starts from the beginning.
 */
class GADUAPI GaduServersManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @short An address and a port. A null address means the hub is to be asked.
     */
    typedef QPair<QHostAddress, int> GaduServer;

    Q_INVOKABLE explicit GaduServersManager(QObject *parent = nullptr);
    virtual ~GaduServersManager();

    /**
     * @short The address the next attempt should use.
     *
     * Asked again once every attempt has been used means somebody has asked for a connection anew,
     * since nothing else brings about a login then; the counting starts over.
     */
    GaduServer getServer();

    /**
     * @short Records where a connection actually got through, and starts the counting over.
     */
    void connectionSucceeded(const GaduServer &server);

    /**
     * @short Counts one attempt as having failed.
     */
    void attemptFailed();

    /**
     * @short Whether anything is left to try.
     */
    bool hasAnotherAttempt() const;

    /**
     * @short How long to wait before the next attempt, in milliseconds.
     */
    int delayBeforeNextAttempt() const;

private:
    GaduServer LastWorkingServer;
    int DirectAttempts;
    int HubAttempts;

    bool isRetryingLastWorkingServer() const;
    void startOver();
};

#endif   // GADU_SERVERS_MANAGER_H
