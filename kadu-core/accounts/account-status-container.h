/*
 * %kadu copyright begin%
 * Copyright 2011, 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#ifndef ACCOUNT_STATUS_CONTAINER_H
#define ACCOUNT_STATUS_CONTAINER_H

#include "status/status.h"
#include "status/storable-status-container.h"

class AccountShared;
class Protocol;

class AccountStatusContainer : public StorableStatusContainer
{
    Q_OBJECT

    AccountShared *Account;

    Status PendingStatus;
    StatusChangeSource PendingStatusSource;
    bool HasPendingStatus;

    void setDisconnectStatus();

public:
    explicit AccountStatusContainer(AccountShared *account);
    virtual ~AccountStatusContainer();

    virtual QString statusContainerName();

    virtual void setStatus(Status newStatus, StatusChangeSource source);
    virtual Status status();

    /**
     * @short Hands the protocol handler a status that was set before it existed.
     *
     * Called when the account gets a protocol handler. Does nothing if no status was held back.
     */
    void applyPendingStatus();

    virtual bool isStatusSettingInProgress();
    virtual int maxDescriptionLength();

    virtual KaduIcon statusIcon();
    virtual KaduIcon statusIcon(const Status &status);

    virtual QList<StatusType> supportedStatusTypes();

public slots:
    void triggerStatusUpdated();
};

#endif   // ACCOUNT_STATUS_CONTAINER_H
