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
#include <injeqt/injeqt.h>
#include <memory>

#include "exports.h"

class QWebEngineProfile;

class ImageStorageService;
class KaduImageSchemeHandler;

/**
 * @addtogroup Web
 * @{
 */

/**
 * @class KaduWebEngineProfile
 * @short Owns the QWebEngineProfile shared by every web view in Kadu.
 *
 * QtWebEngine keeps cache, cookies and settings in a profile, and a profile has to outlive every
 * page created from it. Keeping exactly one, owned by the injector, gives all views a consistent
 * configuration and a lifetime that safely spans theirs.
 *
 * The profile is off-the-record and stores nothing on disk. That is not a simplification: the
 * QtWebKit code it replaces disabled caching outright with
 *
 *     QWebSettings::setMaximumPagesInCache(0);
 *     QWebSettings::setObjectCacheCapacities(0, 0, 0);
 *
 * which makes sense for a chat view -- messages are rendered from local data and a stale cache
 * would only ever show outdated content.
 */
class KADUAPI KaduWebEngineProfile : public QObject
{
    Q_OBJECT

private slots:
    INJEQT_SET void setImageStorageService(ImageStorageService *imageStorageService);

public:
    Q_INVOKABLE explicit KaduWebEngineProfile(QObject *parent = nullptr);
    virtual ~KaduWebEngineProfile();

    /**
     * @return Profile to be passed to every QWebEnginePage created by Kadu.
     *
     * Never null. The returned profile is owned by this object.
     */
    QWebEngineProfile *profile() const;

private:
    std::unique_ptr<QWebEngineProfile> m_profile;
    std::unique_ptr<KaduImageSchemeHandler> m_imageSchemeHandler;
};

/**
 * @}
 */
