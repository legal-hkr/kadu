/*
 * %kadu copyright begin%
 * Copyright 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2009 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2012 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2009 Bartłomiej Zimoń (uzi18@o2.pl)
 * Copyright 2010, 2011, 2012, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "protocol.h"
#include "protocol.moc"

#include "accounts/account-manager.h"
#include "buddies/buddy-manager.h"
#include "chat/chat-manager.h"
#include "contacts/contact-manager.h"
#include "contacts/contact-set.h"
#include "contacts/contact.h"
#include "core/session-service.h"
#include "icons/icons-manager.h"
#include "icons/kadu-icon.h"
#include "misc/kadu-logging.h"
#include "plugin/plugin-injected-factory.h"
#include "protocols/protocol-factory.h"
#include "protocols/protocol-state-machine.h"
#include "roster/roster-service.h"
#include "services/chat-state-service.h"
#include "status/status-type-manager.h"
#include "status/status.h"

#include <QtGui/QTextDocument>

Protocol::Protocol(Account account, ProtocolFactory *factory) : Factory(factory), CurrentAccount(account)
{
}

Protocol::~Protocol()
{
}

void Protocol::setContactManager(ContactManager *contactManager)
{
    m_contactManager = contactManager;
}

void Protocol::setPluginInjectedFactory(PluginInjectedFactory *pluginInjectedFactory)
{
    m_pluginInjectedFactory = pluginInjectedFactory;
}

void Protocol::setSessionService(SessionService *sessionService)
{
    m_sessionService = sessionService;
}

void Protocol::setRosterService(RosterService *const rosterService)
{
    m_rosterService = rosterService;
}

void Protocol::setStatusTypeManager(StatusTypeManager *statusTypeManager)
{
    m_statusTypeManager = statusTypeManager;
}

void Protocol::init()
{
    Machine = m_pluginInjectedFactory->makeInjected<ProtocolStateMachine>(this);
    /*
     * after machine is started we need to re-call changeStatus
     * so proper transition can be called
     *
     * changeStatus was probably called before machine was started by some StatusContainer
     * that just restored status from configuration file
     */
    connect(Machine, SIGNAL(started()), this, SLOT(prepareStateMachine()), Qt::QueuedConnection);

    connect(Machine, SIGNAL(loggingInStateEntered()), this, SLOT(loggingInStateEntered()));
    connect(Machine, SIGNAL(loggedInStateEntered()), this, SLOT(loggedInStateEntered()));
    connect(Machine, SIGNAL(loggingOutStateEntered()), this, SLOT(loggingOutStateEntered()));
    connect(Machine, SIGNAL(loggedOutOnlineStateEntered()), this, SLOT(loggedOutAnyStateEntered()));
    connect(Machine, SIGNAL(loggedOutOfflineStateEntered()), this, SLOT(loggedOutAnyStateEntered()));
    connect(Machine, SIGNAL(wantToLogInStateEntered()), this, SLOT(wantToLogInStateEntered()));
    connect(Machine, SIGNAL(passwordRequiredStateEntered()), this, SLOT(passwordRequiredStateEntered()));
}

ContactManager *Protocol::contactManager() const
{
    return m_contactManager;
}

PluginInjectedFactory *Protocol::pluginInjectedFactory() const
{
    return m_pluginInjectedFactory;
}

StatusTypeManager *Protocol::statusTypeManager() const
{
    return m_statusTypeManager;
}

RosterService *Protocol::rosterService() const
{
    return m_rosterService.data();
}

KaduIcon Protocol::icon()
{
    return Factory->icon();
}

void Protocol::prepareStateMachine()
{
    // Runs once the state machine has started, to replay a status that arrived before it was
    // listening -- which is the normal order when a status container restores from the
    // configuration file before the protocol's plugin is even loaded. If the status is still
    // disconnected here, nothing is replayed and the machine waits for a push that may never come.
    qCDebug(KADU_STATUS_CHANGE) << CurrentAccount.protocolName() << CurrentAccount.id()
                                << "state machine started, pending status is"
                                << (CurrentStatus.isDisconnected() ? "disconnected, nothing to replay"
                                                                   : "connected, replaying it");

    if (!CurrentStatus.isDisconnected())
        emit stateMachineChangeStatus();
}

void Protocol::passwordProvided()
{
    if (CurrentAccount.hasPassword())
    {
        emit stateMachinePasswordAvailable();
        return;
    }

    LoginStatus = Status();
    emit stateMachinePasswordNotAvailable();
}

void Protocol::setAllOffline()
{
    if (m_sessionService->isClosing())
        return;

    Status status;
    Status oldStatus;

    for (auto &contact : m_contactManager->contacts(CurrentAccount))
    {
        oldStatus = contact.currentStatus();

        if (oldStatus != status)
        {
            contact.setCurrentStatus(status);
            emit contactStatusChanged(contact, oldStatus);
        }
    }
}

void Protocol::disconnectedCleanup()
{
    setAllOffline();
}

void Protocol::setStatus(Status status, StatusChangeSource source)
{
    // The guard below is the last point at which an account can stop connecting without a word:
    // no login attempt, no error, and no password prompt either, because the prompt is raised by
    // the state machine and the machine is never asked to move.
    if (SourceStatusChanger == source && !account().hasPassword())
    {
        qCDebug(KADU_STATUS_CHANGE) << CurrentAccount.protocolName() << CurrentAccount.id()
                                    << "ignoring status change from a status changer: no stored password";
        return;
    }

    qCDebug(KADU_STATUS_CHANGE) << CurrentAccount.protocolName() << CurrentAccount.id() << "status change accepted,"
                                << (SourceUser == source ? "from user" : "from status changer");

    LoginStatus = protocolFactory()->adaptStatus(status);
    doSetStatus(LoginStatus);
}

void Protocol::doSetStatus(Status status)
{
    qCDebug(KADU_STATUS_CHANGE) << CurrentAccount.protocolName() << CurrentAccount.id() << "applying status,"
                                << (status.isDisconnected() ? "disconnected: asking the machine to log out"
                                                            : "connected: asking the machine to log in");

    CurrentStatus = status;

    if (!CurrentStatus.isDisconnected())
    {
        emit statusChanged(CurrentAccount, CurrentStatus);
        sendStatusToServer();

        emit stateMachineChangeStatus();
    }
    else
        emit stateMachineLogout();
}

Status Protocol::loginStatus() const
{
    return LoginStatus;
}

Status Protocol::status() const
{
    return CurrentStatus;
}

void Protocol::loggedIn()
{
    emit stateMachineLoggedIn();
}

void Protocol::loggedOut()
{
    emit stateMachineLoggedOut();
}

void Protocol::passwordRequired()
{
    emit stateMachinePasswordRequired();
}

void Protocol::connectionError()
{
    statusChanged(Status());

    emit stateMachineConnectionError();
}

void Protocol::connectionClosed()
{
    doSetStatus(Status());
    statusChanged(Status());

    emit stateMachineConnectionClosed();
}

void Protocol::sslError()
{
    statusChanged(Status());

    emit stateMachineSslError();
}

void Protocol::reconnect()
{
    setStatus(loginStatus(), SourceUser);
}

void Protocol::statusChanged(Status status)
{
    CurrentStatus = status;
    emit statusChanged(CurrentAccount, CurrentStatus);
}

KaduIcon Protocol::statusIcon()
{
    return statusIcon(CurrentStatus);
}

KaduIcon Protocol::statusIcon(const Status &status)
{
    return m_statusTypeManager->statusIcon(statusPixmapPath(), status);
}

void Protocol::loggingInStateEntered()
{
    emit disconnected(CurrentAccount);

    // this may be called from our connection error-handling code, when user wants to be logged in
    // at any cost, so we should assume that we were just disconnected
    // better do some cleanup then
    disconnectedCleanup();

    if (account().id().isEmpty())
    {
        emit stateMachineConnectionClosed();
        return;
    }

    if (!account().hasPassword())
    {
        emit stateMachinePasswordRequired();
        return;
    }

    // just for status icon now, this signal need to be better
    emit statusChanged(CurrentAccount, CurrentStatus);

    // call protocol implementation
    login();
}

void Protocol::loggedInStateEntered()
{
    statusChanged(loginStatus());
    afterLoggedIn();

    emit connected(CurrentAccount);
}

void Protocol::loggingOutStateEntered()
{
    emit disconnected(CurrentAccount);

    // call protocol implementation
    logout();
}

void Protocol::loggedOutAnyStateEntered()
{
    emit disconnected(CurrentAccount);

    disconnectedCleanup();
    statusChanged(loginStatus());
}

void Protocol::wantToLogInStateEntered()
{
    emit disconnected(CurrentAccount);

    disconnectedCleanup();
    statusChanged(Status());

    emit statusChanged(CurrentAccount, Status());
}

void Protocol::passwordRequiredStateEntered()
{
    emit disconnected(CurrentAccount);

    disconnectedCleanup();
    statusChanged(Status());

    emit invalidPassword(CurrentAccount);
}

bool Protocol::isConnected() const
{
    return Machine->isLoggedIn();
}

bool Protocol::isConnecting() const
{
    return Machine->isLoggingIn();
}

bool Protocol::isDisconnecting() const
{
    return Machine->isLoggingOut();
}
