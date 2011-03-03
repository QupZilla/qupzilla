#include "webview.h"
#include "qupzilla.h"
#include "webpage.h"
#include "tabwidget.h"
#include "historymodel.h"
#include "locationbar.h"
#include "downloadmanager.h"
#include "networkmanager.h"
#include "autofillmodel.h"
#include "networkmanagerproxy.h"
#include "mainapplication.h"
#include "tabbar.h"
#include "pluginproxy.h"
#include "webtab.h"

WebView::WebView(QupZilla* mainClass, QWidget *parent)
    : QWebView(parent)
    ,p_QupZilla(mainClass)
    ,m_progress(0)
    ,m_isLoading(false)
    ,m_currentZoom(100)
    ,m_aboutToLoadUrl(QUrl())
    ,m_wantsClose(false)
    ,m_page(new WebPage(this, p_QupZilla))
    //,m_loadingTimer(0)
{
    m_networkProxy = new NetworkManagerProxy(p_QupZilla);
    m_networkProxy->setPrimaryNetworkAccessManager(p_QupZilla->getMainApp()->networkManager());
    m_networkProxy->setPage(m_page);
    m_networkProxy->setView(this);
    m_page->setNetworkAccessManager(m_networkProxy);
    m_page->setView(this);
    setPage(m_page);


    connect(this, SIGNAL(loadStarted()), this, SLOT(loadStarted()));
    connect(this, SIGNAL(loadProgress(int)), this, SLOT(setProgress(int)));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));

    connect(this, SIGNAL(linkClicked(QUrl)), this, SLOT(linkClicked(QUrl)));
    connect(this, SIGNAL(urlChanged(QUrl)), this, SLOT(urlChanged(QUrl)));
    connect(this, SIGNAL(titleChanged(QString)), this, SLOT(titleChanged(QString)));

    connect(this, SIGNAL(statusBarMessage(QString)), p_QupZilla->statusBar(), SLOT(showMessage(QString)));

    connect(page(), SIGNAL(linkHovered(QString, QString, QString)), this, SLOT(linkHovered(QString, QString, QString)));
    connect(page(), SIGNAL(windowCloseRequested()), this, SLOT(closeTab()));
    connect(page(), SIGNAL(downloadRequested(const QNetworkRequest &)), this, SLOT(downloadRequested(const QNetworkRequest &)));

    connect(p_QupZilla->getMainApp()->networkManager(), SIGNAL(finishLoading(bool)), this, SLOT(loadFinished(bool)));
    connect(p_QupZilla->getMainApp()->networkManager(), SIGNAL(wantsFocus(QUrl)), this, SLOT(getFocus(QUrl)));

    //Zoom levels same as in firefox
    m_zoomLevels << 30 << 50 << 67 << 80 << 90 << 100 << 110 << 120 << 133 << 150 << 170 << 200 << 240 << 300;
}

WebPage* WebView::getPage() const
{
    return m_page;
}

bool WebView::isCurrent()
{
    if (!tabWidget())
        return false;
    WebTab* webTab = qobject_cast<WebTab*>(tabWidget()->widget(tabWidget()->currentIndex()));
    if (!webTab)
        return false;

    return (webTab->view() == this);
}

void WebView::urlChanged(const QUrl &url)
{
    if (isCurrent()) {
        emit showUrl(url);
        p_QupZilla->refreshHistory();
    }
    emit changed();
}

void WebView::linkClicked(const QUrl &url)
{
    qDebug() << __FUNCTION__ << "called";
    if (isCurrent())
        emit showUrl(url);
}

void WebView::setProgress(int prog)
{
    m_progress = prog;
    if (isCurrent()) {
        emit showUrl(url());
    }
    checkRss();
}

void WebView::loadStarted()
{
    m_progress = 0;
    m_isLoading = true;

    animationLoading(tabIndex(),true);
    if (title().isNull())
        tabWidget()->setTabText(tabIndex(),tr("Loading..."));

    if (isCurrent()) {
        emit showUrl(url());
    }

    m_currentIp.clear();

//    if (m_loadingTimer)
//        delete m_loadingTimer;
//    m_loadingTimer = new QTimer();
//    connect(m_loadingTimer, SIGNAL(timeout()), this, SLOT(stopAnimation()));
//    m_loadingTimer->start(1000*20); //20 seconds timeout to automatically "stop" loading animation
}

QLabel *WebView::animationLoading(int index, bool addMovie)
{
    if (-1 == index)
        return 0;

    QLabel *loadingAnimation = qobject_cast<QLabel*>(tabWidget()->getTabBar()->tabButton(index, QTabBar::LeftSide));
    if (!loadingAnimation) {
        loadingAnimation = new QLabel(this);
    }
    if (addMovie && !loadingAnimation->movie()) {
        QMovie *movie = new QMovie(":icons/other/progress.gif", QByteArray(), loadingAnimation);
        movie->setSpeed(70);
        loadingAnimation->setMovie(movie);
        movie->start();
    }
    else if (loadingAnimation->movie())
        loadingAnimation->movie()->stop();

    tabWidget()->getTabBar()->setTabButton(index, QTabBar::LeftSide, 0);
    tabWidget()->getTabBar()->setTabButton(index, QTabBar::LeftSide, loadingAnimation);
    return loadingAnimation;
}

void WebView::stopAnimation()
{
    //m_loadingTimer->stop();
    QMovie* mov = animationLoading(tabIndex(), false)->movie();
    if (mov) {
        mov->stop();
        iconChanged();
    }
}

void WebView::setIp(QHostInfo info)
{
    if (info.addresses().isEmpty())
        return;
    m_currentIp = info.hostName() + " ("+info.addresses().at(0).toString()+")";

    if (isCurrent())
        emit ipChanged(m_currentIp);
}

void WebView::loadFinished(bool state)
{
    Q_UNUSED(state);
    if (!animationLoading(tabIndex(), false))
        return;

    if (animationLoading(tabIndex(), false)->movie())
        animationLoading(tabIndex(), false)->movie()->stop();
    if (m_progress>100) qDebug() << "bug"; //cannot be more than 100
    m_isLoading = false;

    p_QupZilla->getMainApp()->history()->addHistoryEntry(this);
    if (isCurrent()) {
        emit showUrl(url());
    }

    iconChanged();

    if (!p_QupZilla->locationBar()->hasFocus())
        setFocus();

    //Fix the bug where sometimes icon is not available at the moment
    if (icon().isNull())
        QTimer::singleShot(1000, this, SLOT(iconChanged()));

    titleChanged(title());
    MainApplication::getInstance()->autoFill()->completePage(this);
    QHostInfo::lookupHost(url().host(), this, SLOT(setIp(QHostInfo)));
}

void WebView::titleChanged(QString title)
{
    if (title.isEmpty()) title = url().host();
    if (title.isEmpty()) title = tr("No Named Page");
    QString title2 = title;
    tabWidget()->setTabToolTip(tabIndex(),title2);

    title2+=" - QupZilla";
    if (isCurrent())
        p_QupZilla->setWindowTitle(title2);

    tabWidget()->setTabText(tabIndex(),title);
}

void WebView::iconChanged()
{
    QIcon icon_ = icon();
    if (!icon_.isNull())
        animationLoading(tabIndex(), false)->setPixmap(icon_.pixmap(16,16));
    else
        animationLoading(tabIndex(), false)->setPixmap(QIcon(":icons/locationbar/unknownpage.png").pixmap(16,16));

    if (isCurrent())
        emit siteIconChanged();
}

void WebView::linkHovered(const QString &link, const QString &title, const QString &content)
{
    Q_UNUSED(title);
    Q_UNUSED(content);
    if (isCurrent()) {
        if (link!="") {
            p_QupZilla->statusBar()->showMessage(link);
        }else{
            isLoading() ? p_QupZilla->statusBar()->showMessage(tr("Loading...")) : p_QupZilla->statusBar()->showMessage(tr("Done"));
        }
    }
    m_hoveredLink = link;
}

TabWidget* WebView::tabWidget() const
{
    QObject *widget = this->parent();
    while (widget) {
        if (TabWidget *tw = qobject_cast<TabWidget*>(widget))
            return tw;
        widget = widget->parent();
    }
    return 0;
}

int WebView::tabIndex() const
{
    TabWidget* tabWid = tabWidget();
    if (!tabWid)
        return -1;

    int i = 0;
    while(qobject_cast<WebTab*>(tabWid->widget(i))->view()!=this) {
        i++;
    }
    return i;
}

QUrl WebView::guessUrlFromString(const QString &string)
{
    QString trimmedString = string.trimmed();

    // Check the most common case of a valid url with scheme and host first
    QUrl url = QUrl::fromEncoded(trimmedString.toUtf8(), QUrl::TolerantMode);
    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty())
        return url;

    // Absolute files that exists
    if (QDir::isAbsolutePath(trimmedString) && QFile::exists(trimmedString))
        return QUrl::fromLocalFile(trimmedString);

    // If the string is missing the scheme or the scheme is not valid prepend a scheme
    QString scheme = url.scheme();
    if (scheme.isEmpty() || scheme.contains(QLatin1Char('.')) || scheme == QLatin1String("localhost")) {
        // Do not do anything for strings such as "foo", only "foo.com"
        int dotIndex = trimmedString.indexOf(QLatin1Char('.'));
        if (dotIndex != -1 || trimmedString.startsWith(QLatin1String("localhost"))) {
            const QString hostscheme = trimmedString.left(dotIndex).toLower();
            QByteArray scheme = (hostscheme == QLatin1String("ftp")) ? "ftp" : "http";
            trimmedString = QLatin1String(scheme) + QLatin1String("://") + trimmedString;
        }
        url = QUrl::fromEncoded(trimmedString.toUtf8(), QUrl::TolerantMode);
    }

    if (url.isValid())
        return url;

    return QUrl();
}

void WebView::checkRss()
{
    QWebFrame* frame = page()->mainFrame();
    QWebElementCollection links = frame->findAllElements("link");
    m_rss.clear();

    for (int i = 0; i<links.count(); i++) {
        QWebElement element = links.at(i);
        //We will show only atom+xml and rss+xml
        if (element.attribute("rel")!="alternate" || (element.attribute("type")!="application/rss+xml" && element.attribute("type")!="application/atom+xml") )
            continue;
        QString title = element.attribute("title");
        QString href = element.attribute("href");
        if (href.isEmpty() || title.isEmpty())
            continue;
        m_rss.append(QPair<QString,QString>(title, href));
    }
}

void WebView::mousePressEvent(QMouseEvent *event)
{
    switch (event->button()) {
    case Qt::XButton1:
        back();
        break;
    case Qt::XButton2:
        forward();
        break;
    case Qt::MiddleButton:
        if (isUrlValid(QUrl(m_hoveredLink)))
            tabWidget()->addView(QUrl(m_hoveredLink),tr("New tab"), TabWidget::NewNotSelectedTab);
        break;
    case Qt::LeftButton:
        if (event->modifiers() == Qt::ControlModifier && isUrlValid(QUrl(m_hoveredLink))) {
            tabWidget()->addView(QUrl(m_hoveredLink),tr("New tab"), TabWidget::NewNotSelectedTab);
            return;
        }
    default:
        QWebView::mousePressEvent(event);
        break;
    }
}

void WebView::mouseReleaseEvent(QMouseEvent* event)
{
    //Workaround for crash in mouseReleaseEvent when closing tab from javascript :/
    if (!m_wantsClose)
        QWebView::mouseReleaseEvent(event);
}

void WebView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Back:
        back();
        event->accept();
        break;
    case Qt::Key_Forward:
        forward();
        event->accept();
        break;
    case Qt::Key_Stop:
        stop();
        event->accept();
        break;
    case Qt::Key_Refresh:
        reload();
        event->accept();
        break;
    default:
        QWebView::keyPressEvent(event);
        return;
    }
}

void WebView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = new QMenu(this);

    QWebHitTestResult r = page()->mainFrame()->hitTestContent(event->pos());

    if (!r.linkUrl().isEmpty()) {
        if (page()->selectedText() == r.linkText())
            findText("");
        menu->addAction(tr("Open link in new window"), this, SLOT(openUrlInNewWindow()))->setData(r.linkUrl());
        menu->addAction(QIcon(":/icons/menu/popup.png"), tr("Open link in new tab"), this, SLOT(openUrlInNewTab()))->setData(r.linkUrl());
        menu->addSeparator();
        menu->addAction(QIcon::fromTheme("user-bookmarks"), tr("Bookmark link"), this, SLOT(bookmarkLink()))->setData(r.linkUrl());
        menu->addAction(QIcon::fromTheme("document-save"), tr("Save link as..."), this, SLOT(downloadLinkToDisk()))->setData(r.linkUrl());
        menu->addAction(tr("Send link..."), this, SLOT(sendLinkByMail()))->setData(r.linkUrl());
        menu->addAction(QIcon::fromTheme("edit-copy"), tr("Copy link address"), this, SLOT(copyLinkToClipboard()))->setData(r.linkUrl());
        menu->addSeparator();
        if (!page()->selectedText().isEmpty())
            menu->addAction(pageAction(QWebPage::Copy));
    }

    if (!r.imageUrl().isEmpty()) {
        if (!menu->isEmpty())
            menu->addSeparator();
        menu->addAction(tr("Show image"), this, SLOT(showImage()))->setData(r.imageUrl());
        menu->addAction(tr("Copy image"), this, SLOT(copyImageToClipboard()))->setData(r.imageUrl());
        menu->addAction(QIcon::fromTheme("edit-copy"), tr("Copy image address"), this, SLOT(copyLinkToClipboard()))->setData(r.imageUrl());
        menu->addSeparator();
        menu->addAction(QIcon::fromTheme("document-save"), tr("Save image as..."), this, SLOT(downloadImageToDisk()));
        menu->addAction(tr("Send image..."), this, SLOT(sendLinkByMail()))->setData(r.linkUrl());
        menu->addSeparator();
        //menu->addAction(tr("Block image"), this, SLOT(blockImage()))->setData(r.imageUrl().toString());
        if (!page()->selectedText().isEmpty())
            menu->addAction(pageAction(QWebPage::Copy));
    }

    QWebElement element = r.element();
       if (!element.isNull() && (element.tagName().toLower() == "input" || element.tagName().toLower() == "textarea")) {
           if (menu->isEmpty()) {
               delete menu;
               menu = page()->createStandardContextMenu();
           }
       }

    if (menu->isEmpty()) {
        QAction* action = menu->addAction(tr("Back"), this, SLOT(back()));
#ifdef Q_WS_X11
        action->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
#else
        action->setIcon(QIcon(":/icons/faenza/back.png"));
#endif
        history()->canGoBack() ? action->setEnabled(true) : action->setEnabled(false);

        action = menu->addAction(tr("Forward"), this, SLOT(forward()));
#ifdef Q_WS_X11
        action->setIcon(style()->standardIcon(QStyle::SP_ArrowForward));
#else
        action->setIcon(QIcon(":/icons/faenza/forward.png"));
#endif
        history()->canGoForward() ? action->setEnabled(true) : action->setEnabled(false);

        menu->addAction(
#ifdef Q_WS_X11
                style()->standardIcon(QStyle::SP_BrowserReload)
#else
                QIcon(":/icons/faenza/reload.png")
#endif
                ,tr("Reload"), this, SLOT(slotReload()));
        action = menu->addAction(
#ifdef Q_WS_X11
                style()->standardIcon(QStyle::SP_BrowserStop)
#else
                QIcon(":/icons/faenza/stop.png")
#endif
                ,tr("Stop"), this, SLOT(stop()));
        isLoading() ? action->setEnabled(true) : action->setEnabled(false);

        menu->addSeparator();
        menu->addAction(QIcon::fromTheme("user-bookmarks"), tr("Bookmark page"), this, SLOT(bookmarkLink()));
        menu->addAction(QIcon::fromTheme("document-save"), tr("Save page as..."), this, SLOT(downloadLinkToDisk()))->setData(url());
        menu->addAction(tr("Send page..."), this, SLOT(sendLinkByMail()))->setData(url());
        menu->addSeparator();
        menu->addAction(QIcon::fromTheme("edit-select-all"), tr("Select all"), this, SLOT(selectAll()));
        if (!page()->selectedText().isEmpty())
            menu->addAction(pageAction(QWebPage::Copy));

        menu->addSeparator();
        menu->addAction(QIcon::fromTheme("text-html"),tr("Show source code"), this, SLOT(showSource()));
        menu->addAction(QIcon::fromTheme("dialog-information"),tr("Show info about site"), this, SLOT(showSiteInfo()))->setData(url());
    }

    MainApplication::getInstance()->plugins()->populateWebViewMenu(menu, this, r);
    menu->addAction(tr("Show Web Inspector"), this, SLOT(showInspector()));

    if (!page()->selectedText().isEmpty()) {
        menu->addSeparator();
        QString selectedText = page()->selectedText();
        selectedText.truncate(20);
        menu->addAction(QIcon(":icons/used/google.png"), tr("Search ")+selectedText+tr("... on Google"), this, SLOT(searchOnGoogle()))->setData(page()->selectedText());
    }

    if (!menu->isEmpty()) {
        //Prevent choosing first option with double rightclick
        QPoint pos = QCursor::pos();
        QPoint p(pos.x(), pos.y()+1);
        menu->exec(p);
        delete menu;
        return;
    }

    QWebView::contextMenuEvent(event);
}

void WebView::addNotification(QWidget *notif)
{
    emit showNotification(notif);
}

void WebView::openUrlInNewTab()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        tabWidget()->addView(action->data().toUrl(), tr("New tab"), TabWidget::NewNotSelectedTab);
    }
}

void WebView::openUrlInNewWindow()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        p_QupZilla->getMainApp()->makeNewWindow(false, action->data().toString());
    }
}

void WebView::sendLinkByMail()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        QDesktopServices::openUrl(QUrl("mailto:?body="+action->data().toString()));
    }
}

void WebView::copyLinkToClipboard()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        QApplication::clipboard()->setText(action->data().toString());
    }
}

void WebView::searchOnGoogle()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        load(QUrl("http://www.google.com/search?client=qupzilla&q="+action->data().toString()));
    }
}

void WebView::selectAll()
{
    triggerPageAction(QWebPage::SelectAll);
}

void WebView::downloadImageToDisk()
{
    triggerPageAction(QWebPage::DownloadImageToDisk);
}

void WebView::copyImageToClipboard()
{
    triggerPageAction(QWebPage::CopyImageToClipboard);
}

void WebView::showImage()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        load(QUrl(action->data().toString()));
    }
}

void WebView::showSource()
{
    p_QupZilla->showSource();
}

void WebView::downloadLinkToDisk()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        QNetworkRequest request(action->data().toUrl());
        DownloadManager* dManager = MainApplication::getInstance()->downManager();
        dManager->download(request);
    }
}

void WebView::downloadRequested(const QNetworkRequest &request)
{
    DownloadManager* dManager = MainApplication::getInstance()->downManager();
    dManager->download(request);
}

void WebView::bookmarkLink()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        if (action->data().isNull())
            p_QupZilla->bookmarkPage();
        else
            p_QupZilla->addBookmark(action->data().toUrl(), action->data().toString());
    }
}

void WebView::showInspector()
{
    p_QupZilla->showInspector();
}

void WebView::showSiteInfo()
{
    p_QupZilla->showPageInfo();
}

void WebView::applyZoom()
{
    setZoomFactor(qreal(m_currentZoom) / 100.0);
}

void WebView::zoomIn()
{
    int i = m_zoomLevels.indexOf(m_currentZoom);

    if (i < m_zoomLevels.count() - 1)
        m_currentZoom = m_zoomLevels[i + 1];
    applyZoom();
}

void WebView::zoomOut()
{
    int i = m_zoomLevels.indexOf(m_currentZoom);

    if (i > 0)
        m_currentZoom = m_zoomLevels[i - 1];
    applyZoom();
}

void WebView::zoomReset()
{
    m_currentZoom = 100;
    applyZoom();
}

void WebView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        int numDegrees = event->delta() / 8;
        int numSteps = numDegrees / 15;
        if (numSteps == 1)
            zoomIn();
        else
            zoomOut();
        event->accept();
        return;
    }
    QWebView::wheelEvent(event);
}

void WebView::getFocus(const QUrl &urla)
{
    if (urla == url())
        tabWidget()->setCurrentWidget(this);
}

void WebView::closeTab()
{
    if (m_wantsClose)
        emit wantsCloseTab(tabIndex());
    else {
        m_wantsClose = true;
        QTimer::singleShot(20, this, SLOT(closeTab()));
    }
}

void WebView::load(QUrl url)
{
    if (url.toString().startsWith("javascript:")) {
        page()->mainFrame()->evaluateJavaScript(url.toString());
        return;
    }
    if (isUrlValid(url)) {
        QWebView::load(url);
        m_aboutToLoadUrl = url;
        return;
    }

#ifdef Q_WS_WIN
    if (QFile::exists(url.path().mid(1))) // From QUrl(file:///C:/Bla/ble/foo.html it returns
                                          // /C:/Bla/ble/foo.html ... so we cut first char
#else
    if (QFile::exists(url.path()))
#endif
        QWebView::load(url);
    else
        QWebView::load(QUrl("http://www.google.com/search?client=qupzilla&q="+url.toString()));
}

QUrl WebView::url() const
{
    QUrl ur = QWebView::url();
    if (ur.isEmpty() && !m_aboutToLoadUrl.isEmpty())
        return m_aboutToLoadUrl;
    return ur;
}

QString WebView::title() const
{
    QString title = QWebView::title();
    if (title.isEmpty())
        return tr("No Named Page");
    return title;
}

void WebView::reload()
{
    if (QWebView::url().isEmpty() && !m_aboutToLoadUrl.isEmpty()) {
        qDebug() << "loading about to load";
        load(m_aboutToLoadUrl);
        return;
    }
    QWebView::reload();
}

bool WebView::isUrlValid(const QUrl &url)
{
    if (url.isValid() && !url.host().isEmpty() && !url.scheme().isEmpty())
        return true;
    return false;
}

WebView::~WebView()
{
    history()->clear();
    delete m_page;
}
