/*
 * %kadu copyright begin%
 * Copyright 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2011 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2010, 2011, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#pragma once

#include <QtCore/QPointer>
#include <QtGui/QClipboard>
#include <QtWebEngineWidgets/QWebEngineView>
#include <injeqt/injeqt.h>

#include "exports.h"

class QUrl;

class ClipboardHtmlTransformerService;
class Configuration;
class IconsManager;
class ImageStorageService;
class KaduWebEnginePage;
class KaduWebEngineProfile;
class UrlHandlerManager;

class KADUAPI KaduWebView : public QWebEngineView
{
    Q_OBJECT

    QPointer<ClipboardHtmlTransformerService> m_clipboardHtmlTransformerService;
    QPointer<Configuration> m_configuration;
    QPointer<IconsManager> m_iconsManager;
    QPointer<ImageStorageService> m_imageStorageService;
    QPointer<UrlHandlerManager> m_urlHandlerManager;

    KaduWebEnginePage *m_page;
    bool IsLoading;

    /**
     * @short Set while a copy is in flight, so the resulting clipboard change can be transformed.
     *
     * QtWebEngine performs the copy in the render process, so the clipboard is not yet populated
     * when the action returns. The transformation has to wait for the clipboard to actually change
     * instead of running straight after the action, as it did under QtWebKit.
     */
    bool m_copyInProgress;

    /**
     * @short CSS most recently passed to setUserFont(), kept so it can be reapplied after a load.
     */
    QString m_userStyleSheet;

    void convertClipboardHtml(QClipboard::Mode mode) const;
    void applyUserStyleSheet();

private slots:
    INJEQT_SET void
    setClipboardHtmlTransformerService(ClipboardHtmlTransformerService *clipboardHtmlTransformerService);
    INJEQT_SET void setConfiguration(Configuration *configuration);
    INJEQT_SET void setIconsManager(IconsManager *iconsManager);
    INJEQT_SET void setImageStorageService(ImageStorageService *imageStorageService);
    INJEQT_SET void setUrlHandlerManager(UrlHandlerManager *urlHandlerManager);
    INJEQT_SET void setWebEngineProfile(KaduWebEngineProfile *webEngineProfile);

    void hyperlinkClicked(const QUrl &anchor) const;
    void loadStartedSlot();
    void loadFinishedSlot(bool success);
    void saveImage();
    void copyRequested();
    void clipboardChanged(QClipboard::Mode mode);
    void selectionChangedSlot();

#ifdef DEBUG_ENABLED
    void runInspector(bool toggled);
#endif

protected:
    Configuration *configuration();

    /**
     * @return Page of this view, created with Kadu's shared profile.
     *
     * Null until the profile has been injected.
     */
    KaduWebEnginePage *kaduPage() const;

    virtual void contextMenuEvent(QContextMenuEvent *e) override;

public:
    explicit KaduWebView(QWidget *parent = nullptr);
    virtual ~KaduWebView();

    ImageStorageService *imageStorageService() const;

    void setUserFont(const QString &fontString, bool force);
    QString userFontStyle(const QFont &font, bool force);
};
