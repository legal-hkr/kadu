/*
 * %kadu copyright begin%
 * Copyright 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2011 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2010, 2011, 2012, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2010, 2011, 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include <QtCore/QFile>
#include <QtCore/QMimeData>
#include <QtCore/QPointer>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtGui/QAction>
#include <QtGui/QClipboard>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QImage>
#include <QtGui/QTextDocument>
#include <QtWebEngineCore/QWebEngineContextMenuRequest>
#include <QtWebEngineCore/QWebEngineScript>
#include <QtWebEngineCore/QWebEngineScriptCollection>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMenu>

#ifdef DEBUG_ENABLED
#include <QtWebEngineWidgets/QWebEngineView>
#endif

#include "web/kadu-web-engine-page.h"
#include "web/kadu-web-engine-profile.h"

#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "gui/services/clipboard-html-transformer-service.h"
#include "icons/icons-manager.h"
#include "protocols/services/chat-image-service.h"
#include "services/image-storage-service.h"
#include "url-handlers/url-handler-manager.h"
#include "windows/message-dialog.h"

#include "kadu-web-view.h"
#include "kadu-web-view.moc"

KaduWebView::KaduWebView(QWidget *parent) : QWebEngineView{parent}, m_page{nullptr}, IsLoading{false}, m_copyInProgress{false}
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAcceptDrops(false);

    // QtWebEngine copies in the render process, so the clipboard is filled asynchronously and the
    // transformation cannot run right after triggering the action. Watch the clipboard instead.
    connect(QApplication::clipboard(), &QClipboard::changed, this, &KaduWebView::clipboardChanged);
}

KaduWebView::~KaduWebView()
{
}

void KaduWebView::setWebEngineProfile(KaduWebEngineProfile *webEngineProfile)
{
    // The page cannot be built in the constructor: it needs the shared profile, and injeqt only
    // supplies that afterwards.
    m_page = new KaduWebEnginePage{webEngineProfile->profile(), this};
    setPage(m_page);

    connect(m_page, &KaduWebEnginePage::linkClicked, this, &KaduWebView::hyperlinkClicked);
    connect(m_page, &QWebEnginePage::loadStarted, this, &KaduWebView::loadStartedSlot);
    connect(m_page, &QWebEnginePage::loadFinished, this, &KaduWebView::loadFinishedSlot);
    connect(m_page, &QWebEnginePage::selectionChanged, this, &KaduWebView::selectionChangedSlot);
    connect(m_page->action(QWebEnginePage::Copy), &QAction::triggered, this, &KaduWebView::copyRequested);

    applyUserStyleSheet();
}

KaduWebEnginePage *KaduWebView::kaduPage() const
{
    return m_page;
}

void KaduWebView::setClipboardHtmlTransformerService(ClipboardHtmlTransformerService *clipboardHtmlTransformerService)
{
    m_clipboardHtmlTransformerService = clipboardHtmlTransformerService;
}

void KaduWebView::setConfiguration(Configuration *configuration)
{
    m_configuration = configuration;
}

void KaduWebView::setIconsManager(IconsManager *iconsManager)
{
    m_iconsManager = iconsManager;
}

void KaduWebView::setImageStorageService(ImageStorageService *imageStorageService)
{
    m_imageStorageService = imageStorageService;
}

void KaduWebView::setUrlHandlerManager(UrlHandlerManager *urlHandlerManager)
{
    m_urlHandlerManager = urlHandlerManager;
}

Configuration *KaduWebView::configuration()
{
    return m_configuration;
}

ImageStorageService *KaduWebView::imageStorageService() const
{
    return m_imageStorageService.data();
}

void KaduWebView::contextMenuEvent(QContextMenuEvent *e)
{
    if (IsLoading)
        return;

    // QWebHitTestResult is gone; QtWebEngine fills in a request object before delivering the event.
    auto const *request = lastContextMenuRequest();
    if (!request)
        return;

    auto const isImage = request->mediaType() == QWebEngineContextMenuRequest::MediaTypeImage;
    auto const isLink = request->linkUrl().isValid();

    QAction *copy = page()->action(QWebEnginePage::Copy);
    copy->setText(tr("Copy"));
    QAction *copyLink = page()->action(QWebEnginePage::CopyLinkToClipboard);
    copyLink->setText(tr("Copy Link Address"));
    copyLink->setEnabled(isLink);
    QAction *copyImage = page()->action(QWebEnginePage::CopyImageToClipboard);
    copyImage->setText(tr("Copy Image"));
    copyImage->setEnabled(isImage);

    // Kadu saves images itself, through ImageStorageService, so this is a plain action rather than
    // QWebEnginePage::DownloadImageToDisk.
    QAction saveImageAction{tr("Save Image"), nullptr};
    saveImageAction.setEnabled(isImage);
    connect(&saveImageAction, &QAction::triggered, this, &KaduWebView::saveImage);

    QMenu popupMenu{this};

    popupMenu.addAction(copy);
    popupMenu.addAction(copyLink);
    popupMenu.addSeparator();
    popupMenu.addAction(copyImage);
    popupMenu.addAction(&saveImageAction);

#ifdef DEBUG_ENABLED
    QAction runInspectorAction{tr("Run Inspector"), nullptr};
    connect(&runInspectorAction, &QAction::triggered, this, &KaduWebView::runInspector);

    popupMenu.addSeparator();
    popupMenu.addAction(&runInspectorAction);
#endif

    popupMenu.exec(e->globalPos());
}

void KaduWebView::hyperlinkClicked(const QUrl &anchor) const
{
    m_urlHandlerManager->openUrl(anchor.toEncoded());
}

void KaduWebView::loadStartedSlot()
{
    IsLoading = true;
}

void KaduWebView::loadFinishedSlot(bool success)
{
    Q_UNUSED(success)

    IsLoading = false;

    // The injected script covers documents loaded from now on; a document that was already loading
    // when setUserFont() ran still needs the style applied by hand.
    applyUserStyleSheet();
}

void KaduWebView::saveImage()
{
    auto const *request = lastContextMenuRequest();
    if (!request)
        return;

    QUrl imageUrl = request->mediaUrl();
    if (m_imageStorageService)
        imageUrl = m_imageStorageService->toFileUrl(imageUrl);

    QString imageFullPath = imageUrl.toLocalFile();
    if (imageFullPath.isEmpty())
        return;

    QImage image;
    QString fileExt = '.' + imageFullPath.section('.', -1);
    bool formatUnknown = false;
    if (fileExt == "." || fileExt.contains('/'))
    {
        // TODO: we'd better guess the file format and not use QImage
        fileExt = ".png";
        formatUnknown = true;

        if (!image.load(imageFullPath))
        {
            MessageDialog::show(
                m_iconsManager->iconByPath(KaduIcon("dialog-warning")), tr("Kadu"), tr("Cannot save this image"));
            return;
        }
    }

    QPointer<QFileDialog> fd = new QFileDialog(this);
    fd->setFileMode(QFileDialog::AnyFile);
    fd->setAcceptMode(QFileDialog::AcceptSave);
    fd->setDirectory(m_configuration->deprecatedApi()->readEntry("Chat", "LastImagePath"));
    fd->setNameFilter(QString("%1 (*%2)").arg(QCoreApplication::translate("ImageDialog", "Images"), fileExt));
    fd->setLabelText(QFileDialog::FileName, imageFullPath.section('/', -1));
    fd->setWindowTitle(tr("Save image"));

    do
    {
        if (fd->exec() != QFileDialog::Accepted)
            break;
        if (fd->selectedFiles().isEmpty())
            break;

        QString file = fd->selectedFiles().at(0);
        if (QFile::exists(file))
        {
            MessageDialog *dialog = MessageDialog::create(
                m_iconsManager->iconByPath(KaduIcon("dialog-question")), tr("Kadu"),
                tr("File already exists. Overwrite?"));
            dialog->addButton(QMessageBox::Yes, tr("Overwrite"));
            dialog->addButton(QMessageBox::No, tr("Cancel"));

            if (dialog->ask())
            {
                QFile removeMe(file);
                if (!removeMe.remove())
                {
                    MessageDialog::show(
                        m_iconsManager->iconByPath(KaduIcon("dialog-warning")), tr("Kadu"),
                        tr("Cannot save image: %1").arg(removeMe.errorString()));
                    break;
                }
            }
            else
                break;
        }

        QString dst = file;
        if (!dst.endsWith(fileExt))
            dst.append(fileExt);

        if (formatUnknown)
        {
            if (!image.save(dst, "PNG"))
            {
                MessageDialog::show(
                    m_iconsManager->iconByPath(KaduIcon("dialog-warning")), tr("Kadu"), tr("Cannot save image"));
                break;
            }
        }
        else
        {
            QFile src(imageFullPath);
            if (!src.copy(dst))
            {
                MessageDialog::show(
                    m_iconsManager->iconByPath(KaduIcon("dialog-warning")), tr("Kadu"),
                    tr("Cannot save image: %1").arg(src.errorString()));
                break;
            }
        }

        m_configuration->deprecatedApi()->writeEntry("Chat", "LastImagePath", fd->directory().absolutePath());
    } while (false);

    delete fd.data();
}

#ifdef DEBUG_ENABLED
void KaduWebView::runInspector(bool toggled)
{
    Q_UNUSED(toggled)

    if (!m_page)
        return;

    // QWebInspector is gone. QtWebEngine drives developer tools through a second page, which any
    // view can display -- here a standalone window that closes with itself.
    auto *inspectorView = new QWebEngineView{};
    inspectorView->setAttribute(Qt::WA_DeleteOnClose);
    inspectorView->setWindowTitle(tr("Kadu Inspector"));
    inspectorView->resize(900, 600);

    m_page->setDevToolsPage(inspectorView->page());
    inspectorView->show();
}
#endif

void KaduWebView::copyRequested()
{
    // Only arm the transformation. QtWebEngine has not written the clipboard yet at this point.
    m_copyInProgress = true;
}

void KaduWebView::clipboardChanged(QClipboard::Mode mode)
{
    if (mode != QClipboard::Clipboard || !m_copyInProgress)
        return;

    // Cleared before transforming: convertClipboardHtml() writes the clipboard again and would
    // otherwise re-enter here forever.
    m_copyInProgress = false;
    convertClipboardHtml(QClipboard::Clipboard);
}

void KaduWebView::selectionChangedSlot()
{
#if defined(Q_OS_UNIX)
    // Under QtWebKit this hung off mouseReleaseEvent(). QtWebEngine renders into a native child
    // widget, so the view never sees those events and the selection signal takes their place.
    if (!selectedText().isEmpty())
        convertClipboardHtml(QClipboard::Selection);
#endif
}

// taken from Psi+'s webkit patch, SVN rev. 2638, and slightly modified
void KaduWebView::convertClipboardHtml(QClipboard::Mode mode) const
{
    auto html = QApplication::clipboard()->mimeData(mode)->html();
    html = m_clipboardHtmlTransformerService->transform(html);

    QTextDocument document;
    document.setHtml(html);
    QMimeData *data = new QMimeData();
    data->setHtml(html);

    // remove OBJECT REPLACEMENT CHARACTER
    // see http://www.kadu.im/redmine/issues/2490
    data->setText(document.toPlainText().remove(QChar(0xfffc)));
    QApplication::clipboard()->setMimeData(data, mode);
}

namespace
{
const auto UserStyleSheetScriptName = QStringLiteral("kadu-user-stylesheet");

/**
 * @short Wrap text in a single-quoted JavaScript string literal.
 *
 * The style sheet contains quotes -- font-family:"Noto Sans" -- so it cannot simply be pasted into
 * generated script.
 */
QString toJsStringLiteral(const QString &text)
{
    auto escaped = text;
    escaped.replace('\\', QStringLiteral("\\\\"));
    escaped.replace('\'', QStringLiteral("\\'"));
    escaped.replace('\n', QStringLiteral("\\n"));
    escaped.replace('\r', QStringLiteral("\\r"));
    return '\'' + escaped + '\'';
}

/**
 * @short Script that puts the given CSS into a style element, replacing any previous one.
 */
QString userStyleSheetScript(const QString &css)
{
    return QStringLiteral(
               "(function() {"
               "  var id = 'kadu-user-stylesheet';"
               "  var element = document.getElementById(id);"
               "  if (!element) {"
               "    element = document.createElement('style');"
               "    element.id = id;"
               "    (document.head || document.documentElement).appendChild(element);"
               "  }"
               "  element.textContent = %1;"
               "})();")
        .arg(toJsStringLiteral(css));
}
}

void KaduWebView::setUserFont(const QString &fontString, bool force)
{
    QString style;

    if (fontString.isEmpty())
        style = "* { font-family: sans-serif; }";
    else
    {
        QFont font;
        font.fromString(fontString);
        style = QString("* { %1 }").arg(userFontStyle(font, force));
    }

    style.append(
        "\
		img.scalable { max-width: 80%; }\
		img.scalable.unscaled { max-width: none; }\
	");

    m_userStyleSheet = style;
    applyUserStyleSheet();
}

void KaduWebView::applyUserStyleSheet()
{
    if (!m_page || m_userStyleSheet.isEmpty())
        return;

    // QWebEngineSettings has no setUserStyleSheetUrl(); the style is injected as a script that runs
    // on every document this page loads.
    auto const script = userStyleSheetScript(m_userStyleSheet);

    auto &scripts = m_page->scripts();
    for (auto const &existing : scripts.find(UserStyleSheetScriptName))
        scripts.remove(existing);

    QWebEngineScript styleSheetScript;
    styleSheetScript.setName(UserStyleSheetScriptName);
    styleSheetScript.setInjectionPoint(QWebEngineScript::DocumentReady);
    styleSheetScript.setWorldId(QWebEngineScript::ApplicationWorld);
    styleSheetScript.setRunsOnSubFrames(true);
    styleSheetScript.setSourceCode(script);
    scripts.insert(styleSheetScript);

    // Covers the document that is already loaded; the script above only fires on the next one.
    m_page->runJavaScript(script, QWebEngineScript::ApplicationWorld);
}

QString KaduWebView::userFontStyle(const QFont &font, bool force)
{
    QString style = "font-family:\"" + font.family() + "\",sans-serif" + (force ? " !important;" : ";");
    if (force && font.pointSize() != -1)
        style += QString(" font-size:%1pt;").arg(font.pointSize());
    return style;
}
