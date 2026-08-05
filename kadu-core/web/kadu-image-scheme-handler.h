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

#include <QtCore/QPointer>
#include <QtWebEngineCore/QWebEngineUrlSchemeHandler>

#include "exports.h"

class ImageStorageService;

/**
 * @addtogroup Web
 * @{
 */

/**
 * @class KaduImageSchemeHandler
 * @short Serves images received in messages, addressed as kaduimg:///<id>.
 *
 * Images that arrive in a conversation are written to Kadu's image storage and referenced from the
 * rendered message by a kaduimg: URL, which ImageStorageService maps to a file.
 *
 * Under QtWebKit that mapping was done by swapping the page's QNetworkAccessManager for one that
 * rewrote the request. QtWebEngine does not use QNetworkAccessManager at all, and its equivalent is
 * a handler for a registered custom scheme -- so the URL is resolved and the file served from here
 * instead.
 *
 * The scheme itself has to be registered before QtWebEngine starts; see registerScheme().
 */
class KADUAPI KaduImageSchemeHandler : public QWebEngineUrlSchemeHandler
{
    Q_OBJECT

public:
    /**
     * @short Name of the scheme, as it appears in rendered messages.
     */
    static QByteArray schemeName();

    /**
     * @short Register the scheme with QtWebEngine.
     *
     * Must be called before QApplication is constructed: QtWebEngine reads the scheme registry
     * while starting up and refuses schemes added afterwards.
     */
    static void registerScheme();

    explicit KaduImageSchemeHandler(ImageStorageService *imageStorageService, QObject *parent = nullptr);
    virtual ~KaduImageSchemeHandler();

    virtual void requestStarted(QWebEngineUrlRequestJob *job) override;

private:
    QPointer<ImageStorageService> m_imageStorageService;
};

/**
 * @}
 */
