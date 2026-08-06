/*
 * %kadu copyright begin%
 * Copyright 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "icons/kadu-icon.h"
#include "misc/kadu-logging.h"
#include "protocols/protocol-factory.h"
#include "protocols/protocol.h"

#include "account-status-container.h"
#include "account-status-container.moc"

AccountStatusContainer::AccountStatusContainer(AccountShared *account)
        : StorableStatusContainer(account), Account(account), PendingStatusSource(SourceStatusChanger),
          HasPendingStatus(false)
{
}

AccountStatusContainer::~AccountStatusContainer()
{
}

QString AccountStatusContainer::statusContainerName()
{
    return Account->Id;
}

void AccountStatusContainer::setStatus(Status newStatus, StatusChangeSource source)
{
    if (Account->ProtocolHandler)
        Account->ProtocolHandler->setStatus(newStatus, source);
    else
    {
        // The status this account belongs to is restored from the configuration file at startup,
        // which happens before the protocol's plugin is necessarily loaded. Dropping the status
        // here used to strand such an account for the whole session: nothing sends it again, because
        // StatusChangerManager only pushes a status down when it changes, and it has not changed.
        // The account then sat in the logged-out-online state -- not connected, never retrying, and
        // saying nothing, while the status widget kept reporting the identity as available.
        qCDebug(KADU_STATUS_CHANGE) << Account->Id << "has no protocol handler yet, holding the status back";
        PendingStatus = newStatus;
        PendingStatusSource = source;
        HasPendingStatus = true;
    }

    if (newStatus.isDisconnected() && !Account->rememberPassword())
        Account->setPassword("");
}

void AccountStatusContainer::applyPendingStatus()
{
    if (!HasPendingStatus || !Account->ProtocolHandler)
        return;

    qCDebug(KADU_STATUS_CHANGE) << Account->Id << "protocol handler is up, delivering the held back status";
    HasPendingStatus = false;
    Account->ProtocolHandler->setStatus(PendingStatus, PendingStatusSource);
}

Status AccountStatusContainer::status()
{
    if (Account->ProtocolHandler)
        return Account->ProtocolHandler->status();
    else
        return Status();
}

bool AccountStatusContainer::isStatusSettingInProgress()
{
    if (Account->ProtocolHandler)
        return Account->ProtocolHandler->isConnecting();
    else
        return false;
}

int AccountStatusContainer::maxDescriptionLength()
{
    if (Account->ProtocolHandler)
        return Account->ProtocolHandler->maxDescriptionLength();
    else
        return 0;
}

KaduIcon AccountStatusContainer::statusIcon()
{
    return statusIcon(status());
}

KaduIcon AccountStatusContainer::statusIcon(const Status &status)
{
    if (Account->ProtocolHandler)
        return Account->ProtocolHandler->statusIcon(status);
    else
        return KaduIcon();
}

QList<StatusType> AccountStatusContainer::supportedStatusTypes()
{
    if (Account->ProtocolHandler)
        return Account->ProtocolHandler->protocolFactory()->supportedStatusTypes();
    else
        return QList<StatusType>();
}

void AccountStatusContainer::triggerStatusUpdated()
{
    emit statusUpdated(this);
}
