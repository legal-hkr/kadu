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

#include <QtCore/QObject>
#include <QtCore/QVariantMap>
#include <QtGui/QPixmap>

/**
 * @short Takes a screenshot through org.freedesktop.portal.Screenshot.
 *
 * A Wayland client cannot read the contents of the screen, or of anyone else's window: the
 * compositor holds those pixels and hands them over only through the desktop's own portal, which
 * asks the user first. So the picture arrives later and may not arrive at all, and both the mode
 * without a picker and the one with it are the desktop's to implement rather than ours.
 */
class PortalScreenshot : public QObject
{
    Q_OBJECT

public:
    explicit PortalScreenshot(QObject *parent = nullptr);
    virtual ~PortalScreenshot();

    /**
     * @short Asks the desktop for a screenshot.
     * @param interactive let the user choose an area or a window using the desktop's own tool
     *
     * Answers exactly once, through taken() or through failed().
     */
    void take(bool interactive);

signals:
    void taken(QPixmap screenshot);
    void failed(const QString &errorMessage);

private slots:
    void response(uint code, const QVariantMap &results);

private:
    QString RequestPath;

    bool listenForResponse();
    void stopListening();
    void finishWithError(const QString &errorMessage);
};
