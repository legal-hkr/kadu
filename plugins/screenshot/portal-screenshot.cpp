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

#include "portal-screenshot.h"
#include "portal-screenshot.moc"

#include <QtCore/QFile>
#include <QtCore/QUrl>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusObjectPath>
#include <QtDBus/QDBusReply>

namespace
{
const auto PortalService = QStringLiteral("org.freedesktop.portal.Desktop");
const auto PortalPath = QStringLiteral("/org/freedesktop/portal/desktop");
const auto ScreenshotInterface = QStringLiteral("org.freedesktop.portal.Screenshot");
const auto RequestInterface = QStringLiteral("org.freedesktop.portal.Request");

// The portal answers a request that is still being set up, so the call itself must not wait for
// the user: it returns the path of the request object, and the picture arrives on it later.
constexpr int CallTimeoutMsec = 15000;
}

PortalScreenshot::PortalScreenshot(QObject *parent) : QObject{parent}
{
}

PortalScreenshot::~PortalScreenshot()
{
}

void PortalScreenshot::take(bool interactive)
{
    auto bus = QDBusConnection::sessionBus();
    if (!bus.isConnected())
    {
        finishWithError(tr("No session bus, so the desktop cannot be asked for a screenshot."));
        return;
    }

    auto call = QDBusMessage::createMethodCall(PortalService, PortalPath, ScreenshotInterface,
                                               QStringLiteral("Screenshot"));

    QVariantMap options;
    options.insert(QStringLiteral("interactive"), interactive);

    // The window the desktop should attach its own dialog to. Wayland identifiers are obtained
    // through a separate exporting protocol, and getting one wrong is worse than leaving it out:
    // an empty string simply means the dialog is not parented.
    call << QString{} << options;

    QDBusReply<QDBusObjectPath> reply = bus.call(call, QDBus::Block, CallTimeoutMsec);
    if (!reply.isValid())
    {
        finishWithError(tr("The desktop refused the screenshot request: %1").arg(reply.error().message()));
        return;
    }

    RequestPath = reply.value().path();
    if (!bus.connect(QString{}, RequestPath, RequestInterface, QStringLiteral("Response"), this,
                     SLOT(response(uint, QVariantMap))))
    {
        RequestPath.clear();
        finishWithError(tr("Could not listen for the desktop's answer to the screenshot request."));
    }
}

void PortalScreenshot::response(uint code, const QVariantMap &results)
{
    QDBusConnection::sessionBus().disconnect(QString{}, RequestPath, RequestInterface, QStringLiteral("Response"),
                                             this, SLOT(response(uint, QVariantMap)));
    RequestPath.clear();

    // 0 succeeded, 1 the user cancelled, 2 something else went wrong. A cancellation is a decision,
    // not a fault, so it carries no message.
    if (1 == code)
    {
        emit failed(QString{});
        return;
    }
    if (0 != code)
    {
        finishWithError(tr("The desktop could not take the screenshot."));
        return;
    }

    auto const uri = results.value(QStringLiteral("uri")).toString();
    auto const fileName = QUrl{uri}.toLocalFile();
    if (fileName.isEmpty())
    {
        finishWithError(tr("The desktop reported a screenshot but gave no file."));
        return;
    }

    QPixmap screenshot;
    auto const loaded = screenshot.load(fileName);

    // The file belongs to us from here on, and nothing else will remove it.
    QFile::remove(fileName);

    if (!loaded)
    {
        finishWithError(tr("The screenshot taken by the desktop could not be read."));
        return;
    }

    emit taken(screenshot);
}

void PortalScreenshot::finishWithError(const QString &errorMessage)
{
    emit failed(errorMessage);
}
