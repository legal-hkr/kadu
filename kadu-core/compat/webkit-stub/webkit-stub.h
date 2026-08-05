/*
 * Qt6 port scaffolding -- NOT a working WebKit implementation.
 *
 * QtWebKit does not exist in Qt6 and Kadu's chat view is written directly
 * against its C++ API. These stubs let the rest of kadu-core compile, link
 * and run while the chat layer is rewritten; every call is a no-op and the
 * chat view renders nothing.
 *
 * The QtWebKit/ and QtWebKitWidgets/ directories next to this file forward
 * to this header, so no source file needs its includes changed. Delete the
 * whole compat/webkit-stub directory once the chat layer targets a real
 * backend.
 *
 * Note that the chat layer cannot simply be ported: it drives the DOM from
 * C++ via QWebFrame::documentElement(), and QtWebEngine offers no C++ DOM
 * API at all. Those call sites have to become JavaScript, which is a rewrite
 * rather than a port.
 */

#pragma once

#include <QtCore/QObject>
#include <QtCore/QPoint>
#include <QtCore/QSize>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QWidget>

class QNetworkAccessManager;
class QNetworkRequest;
class QWebFrame;
class QWebPage;

class QWebHistory
{
public:
    void clear()
    {
    }
    bool canGoBack() const
    {
        return false;
    }
    bool canGoForward() const
    {
        return false;
    }
};

class QWebSettings
{
public:
    enum WebAttribute
    {
        AutoLoadImages,
        JavascriptEnabled,
        JavaEnabled,
        PluginsEnabled,
        DeveloperExtrasEnabled,
        LocalStorageEnabled,
        LocalContentCanAccessFileUrls,
        LocalContentCanAccessRemoteUrls
    };

    enum FontFamily
    {
        StandardFont,
        FixedFont,
        SerifFont,
        SansSerifFont,
        CursiveFont,
        FantasyFont
    };

    void setAttribute(WebAttribute, bool)
    {
    }
    bool testAttribute(WebAttribute) const
    {
        return false;
    }
    void setUserStyleSheetUrl(const QUrl &)
    {
    }
    void setFontFamily(FontFamily, const QString &)
    {
    }

    static QWebSettings *globalSettings()
    {
        static QWebSettings settings;
        return &settings;
    }
    static void setMaximumPagesInCache(int)
    {
    }
    static void setObjectCacheCapacities(int, int, int)
    {
    }
};

/*
 * QWebElement modelled the live DOM. Nothing in Qt6 replaces it, so every
 * accessor returns an empty value and every mutator discards its input.
 */
class QWebElement
{
public:
    bool isNull() const
    {
        return true;
    }
    QString tagName() const
    {
        return QString{};
    }
    QWebElement findFirst(const QString &) const
    {
        return QWebElement{};
    }
    QList<QWebElement> childNodes() const
    {
        return QList<QWebElement>{};
    }
    QString attribute(const QString &, const QString &defaultValue = QString{}) const
    {
        return defaultValue;
    }
    void setAttribute(const QString &, const QString &)
    {
    }
    void appendInside(const QString &)
    {
    }
    void setInnerXml(const QString &)
    {
    }
    QString toInnerXml() const
    {
        return QString{};
    }
    void removeFromDocument()
    {
    }
};

class QWebHitTestResult
{
public:
    bool isNull() const
    {
        return true;
    }
    QUrl imageUrl() const
    {
        return QUrl{};
    }
    QUrl linkUrl() const
    {
        return QUrl{};
    }
    QString linkText() const
    {
        return QString{};
    }
    QWebElement element() const
    {
        return QWebElement{};
    }
    QWebElement enclosingBlockElement() const
    {
        return QWebElement{};
    }
};

class QWebFrame : public QObject
{
    Q_OBJECT

public:
    explicit QWebFrame(QWebPage *page = nullptr);
    ~QWebFrame() override;

    QVariant evaluateJavaScript(const QString &scriptSource);

    QWebElement documentElement() const;
    QWebElement findFirstElement(const QString &selectorQuery) const;

    void setHtml(const QString &html, const QUrl &baseUrl = QUrl{});
    QString toHtml() const;

    void addToJavaScriptWindowObject(const QString &name, QObject *object);

    void scrollToAnchor(const QString &anchor);
    QPoint scrollPosition() const;
    void setScrollPosition(const QPoint &position);
    int scrollBarValue(Qt::Orientation orientation) const;
    int scrollBarMaximum(Qt::Orientation orientation) const;
    void setScrollBarValue(Qt::Orientation orientation, int value);

    QWebHitTestResult hitTestContent(const QPoint &position) const;

    QWebPage *page() const;

Q_SIGNALS:
    void javaScriptWindowObjectCleared();
    void contentsSizeChanged(const QSize &size);

private:
    QWebPage *m_page;
};

class QWebPage : public QObject
{
    Q_OBJECT

public:
    enum WebAction
    {
        NoWebAction,
        Copy,
        CopyImageToClipboard,
        CopyLinkToClipboard,
        DownloadImageToDisk,
        DownloadLinkToDisk
    };

    enum LinkDelegationPolicy
    {
        DontDelegateLinks,
        DelegateExternalLinks,
        DelegateAllLinks
    };

    enum FindFlag
    {
        FindBackward = 0x01,
        FindCaseSensitively = 0x02,
        FindWrapsAroundDocument = 0x04,
        HighlightAllOccurrences = 0x08
    };
    Q_DECLARE_FLAGS(FindFlags, FindFlag)

    explicit QWebPage(QObject *parent = nullptr);
    ~QWebPage() override;

    QWebFrame *mainFrame() const;
    QWebFrame *currentFrame() const;
    QWebSettings *settings() const;
    QWebHistory *history() const;

    QAction *action(WebAction action) const;
    void triggerAction(WebAction action, bool checked = false);

    QNetworkAccessManager *networkAccessManager() const;
    void setNetworkAccessManager(QNetworkAccessManager *manager);

    void setLinkDelegationPolicy(LinkDelegationPolicy policy);

    QSize viewportSize() const;
    void setViewportSize(const QSize &size) const;

    QString selectedText() const;

    bool findText(const QString &subString, FindFlags options = FindFlags());

Q_SIGNALS:
    void linkClicked(const QUrl &url);
    void downloadRequested(const QNetworkRequest &request);

private:
    QWebFrame *m_frame;
    QNetworkAccessManager *m_networkAccessManager;
    mutable QWebHistory m_history;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QWebPage::FindFlags)

class QWebView : public QWidget
{
    Q_OBJECT

public:
    explicit QWebView(QWidget *parent = nullptr);
    ~QWebView() override;

    QWebPage *page() const;
    void setPage(QWebPage *page);

    void setHtml(const QString &html, const QUrl &baseUrl = QUrl{});
    void setUrl(const QUrl &url);
    void load(const QUrl &url);

    QWebSettings *settings() const;
    QWebHistory *history() const;

    QAction *pageAction(QWebPage::WebAction action) const;
    void triggerPageAction(QWebPage::WebAction action, bool checked = false);

    QString selectedText() const;

    bool findText(const QString &subString, QWebPage::FindFlags options = QWebPage::FindFlags());

Q_SIGNALS:
    void loadStarted();
    void loadFinished(bool ok);
    void linkClicked(const QUrl &url);

private:
    QWebPage *m_page;
};

class QWebInspector : public QWidget
{
    Q_OBJECT

public:
    explicit QWebInspector(QWidget *parent = nullptr);
    ~QWebInspector() override;

    void setPage(QWebPage *page);
};
