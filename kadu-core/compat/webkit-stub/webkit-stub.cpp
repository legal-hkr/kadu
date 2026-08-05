/*
 * Qt6 port scaffolding -- NOT a working WebKit implementation.
 * See webkit-stub.h for why this exists.
 */

#include "webkit-stub.h"

QWebFrame::QWebFrame(QWebPage *page) : QObject{page}, m_page{page}
{
}

QWebFrame::~QWebFrame()
{
}

QVariant QWebFrame::evaluateJavaScript(const QString &scriptSource)
{
    Q_UNUSED(scriptSource)
    return QVariant{};
}

QWebElement QWebFrame::documentElement() const
{
    return QWebElement{};
}

QWebElement QWebFrame::findFirstElement(const QString &selectorQuery) const
{
    Q_UNUSED(selectorQuery)
    return QWebElement{};
}

void QWebFrame::setHtml(const QString &html, const QUrl &baseUrl)
{
    Q_UNUSED(html)
    Q_UNUSED(baseUrl)
}

QString QWebFrame::toHtml() const
{
    return QString{};
}

void QWebFrame::addToJavaScriptWindowObject(const QString &name, QObject *object)
{
    Q_UNUSED(name)
    Q_UNUSED(object)
}

void QWebFrame::setScrollBarPolicy(Qt::Orientation orientation, Qt::ScrollBarPolicy policy)
{
    Q_UNUSED(orientation)
    Q_UNUSED(policy)
}

void QWebFrame::scrollToAnchor(const QString &anchor)
{
    Q_UNUSED(anchor)
}

QPoint QWebFrame::scrollPosition() const
{
    return QPoint{};
}

void QWebFrame::setScrollPosition(const QPoint &position)
{
    Q_UNUSED(position)
}

int QWebFrame::scrollBarValue(Qt::Orientation orientation) const
{
    Q_UNUSED(orientation)
    return 0;
}

int QWebFrame::scrollBarMaximum(Qt::Orientation orientation) const
{
    Q_UNUSED(orientation)
    return 0;
}

void QWebFrame::setScrollBarValue(Qt::Orientation orientation, int value)
{
    Q_UNUSED(orientation)
    Q_UNUSED(value)
}

QWebHitTestResult QWebFrame::hitTestContent(const QPoint &position) const
{
    Q_UNUSED(position)
    return QWebHitTestResult{};
}

QSize QWebFrame::contentsSize() const
{
    return QSize{};
}

QWebPage *QWebFrame::page() const
{
    return m_page;
}

QWebPage::QWebPage(QObject *parent)
        : QObject{parent}, m_frame{new QWebFrame{this}}, m_networkAccessManager{nullptr}
{
}

QWebPage::~QWebPage()
{
}

QWebFrame *QWebPage::mainFrame() const
{
    return m_frame;
}

QWebFrame *QWebPage::currentFrame() const
{
    return m_frame;
}

QWebSettings *QWebPage::settings() const
{
    return QWebSettings::globalSettings();
}

QWebHistory *QWebPage::history() const
{
    return &m_history;
}

QAction *QWebPage::action(WebAction action) const
{
    Q_UNUSED(action)
    return nullptr;
}

void QWebPage::triggerAction(WebAction action, bool checked)
{
    Q_UNUSED(action)
    Q_UNUSED(checked)
}

QNetworkAccessManager *QWebPage::networkAccessManager() const
{
    return m_networkAccessManager;
}

void QWebPage::setNetworkAccessManager(QNetworkAccessManager *manager)
{
    m_networkAccessManager = manager;
}

void QWebPage::setLinkDelegationPolicy(LinkDelegationPolicy policy)
{
    Q_UNUSED(policy)
}

QSize QWebPage::viewportSize() const
{
    return QSize{};
}

void QWebPage::setViewportSize(const QSize &size) const
{
    Q_UNUSED(size)
}

QString QWebPage::selectedText() const
{
    return QString{};
}

void QWebPage::setPalette(const QPalette &palette)
{
    Q_UNUSED(palette)
}

bool QWebPage::findText(const QString &subString, FindFlags options)
{
    Q_UNUSED(subString)
    Q_UNUSED(options)
    return false;
}

QWebView::QWebView(QWidget *parent) : QWidget{parent}, m_page{new QWebPage{this}}
{
}

QWebView::~QWebView()
{
}

QWebPage *QWebView::page() const
{
    return m_page;
}

void QWebView::setPage(QWebPage *page)
{
    m_page = page;
}

void QWebView::setRenderHints(QPainter::RenderHints hints)
{
    Q_UNUSED(hints)
}

void QWebView::setHtml(const QString &html, const QUrl &baseUrl)
{
    Q_UNUSED(html)
    Q_UNUSED(baseUrl)
}

void QWebView::setUrl(const QUrl &url)
{
    Q_UNUSED(url)
}

void QWebView::load(const QUrl &url)
{
    Q_UNUSED(url)
}

QWebSettings *QWebView::settings() const
{
    return QWebSettings::globalSettings();
}

QWebHistory *QWebView::history() const
{
    return m_page ? m_page->history() : nullptr;
}

QAction *QWebView::pageAction(QWebPage::WebAction action) const
{
    Q_UNUSED(action)
    return nullptr;
}

void QWebView::triggerPageAction(QWebPage::WebAction action, bool checked)
{
    Q_UNUSED(action)
    Q_UNUSED(checked)
}

QString QWebView::selectedText() const
{
    return QString{};
}

bool QWebView::findText(const QString &subString, QWebPage::FindFlags options)
{
    Q_UNUSED(subString)
    Q_UNUSED(options)
    return false;
}

QWebInspector::QWebInspector(QWidget *parent) : QWidget{parent}
{
}

QWebInspector::~QWebInspector()
{
}

void QWebInspector::setPage(QWebPage *page)
{
    Q_UNUSED(page)
}
