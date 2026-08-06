/*
 * %kadu copyright begin%
 * Copyright 2011 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2012, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include <QtCore/QDebug>
#include <QtCore/QSysInfo>
#include <QtNetwork/QNetworkInformation>

#include "network-manager-qt.h"
#include "network-manager-qt.moc"

namespace
{
/**
 * @short Whether a reachability value should stop Kadu from using the network.
 *
 * Only an explicit Disconnected counts as offline. Unknown means the backend cannot tell, and
 * Local and Site mean it sees a network but has not confirmed a route to the internet -- which is
 * what a NetworkManager backend reports whenever its connectivity check is disabled or blocked,
 * and that is common. Treating those as offline tears down a connection that works perfectly
 * well: the protocol finds out whether it can reach its servers by trying.
 */
bool isReachable(QNetworkInformation::Reachability reachability)
{
    return reachability != QNetworkInformation::Reachability::Disconnected;
}
}

NetworkManagerQt::NetworkManagerQt(QObject *parent) : NetworkManager{parent}
{
    HasReachabilityBackend = QNetworkInformation::loadDefaultBackend() && QNetworkInformation::instance();

    if (!HasReachabilityBackend)
    {
        qDebug("network: no QNetworkInformation backend, assuming the network is available");
        return;
    }

    auto *information = QNetworkInformation::instance();
    qDebug(
        "network: QNetworkInformation backend \"%s\", reachability %d",
        qPrintable(information->backendName()), int(information->reachability()));

    connect(
        information, &QNetworkInformation::reachabilityChanged, this,
        [this](QNetworkInformation::Reachability reachability) {
            qDebug("network: reachability changed to %d, online=%d", int(reachability), int(isReachable(reachability)));
            onlineStateChanged(isReachable(reachability));
        });
}

NetworkManagerQt::~NetworkManagerQt()
{
}

bool NetworkManagerQt::isOnline()
{
    if (!HasReachabilityBackend)
        return true;
    return isReachable(QNetworkInformation::instance()->reachability());
}

void NetworkManagerQt::forceOnline()
{
    onlineStateChanged(true);
}
