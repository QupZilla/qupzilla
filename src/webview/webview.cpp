/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
* ============================================================ */
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
#include "networkmanager.h"
#include "mainapplication.h"
#include "tabbar.h"
#include "pluginproxy.h"
#include "iconprovider.h"
#include "webtab.h"
#include "statusbarmessage.h"
#include "progressbar.h"
#include "navigationbar.h"
#include "searchenginesmanager.h"

WebView::WebView(QupZilla* mainClass, WebTab* webTab)
    : QWebView(webTab)
    , p_QupZilla(mainClass)
    , m_aboutToLoadUrl(QUrl())
    , m_lastUrl(QUrl())
    , m_progress(0)
    , m_currentZoom(100)
    , m_page(new WebPage(this, p_QupZilla))
    , m_webTab(webTab)
    , m_locationBar(0)
    , m_menu(new QMenu(this))
    , m_clickedFrame(0)
    , m_mouseTrack(false)
    , m_navigationVisible(false)
    , m_mouseWheelEnabled(true)
    , m_wantsClose(false)
    , m_isLoading(false)
    , m_hasRss(false)
    , m_rssChecked(false)
//    , m_loadingTimer(0)
{
    m_networkProxy = new NetworkManagerProxy(this);
    m_networkProxy->setPrimaryNetworkAccessManager(mApp->networkManager());
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
    connect(this, SIGNAL(titleChanged(QString)), this, SLOT(titleChanged()));
    connect(this, SIGNAL(iconChanged()), this, SLOT(slotIconChanged()));

    connect(this, SIGNAL(statusBarMessage(QString)), p_QupZilla->statusBar(), SLOT(showMessage(QString)));

    connect(page(), SIGNAL(linkHovered(QString, QString, QString)), this, SLOT(linkHovered(QString, QString, QString)));
    connect(page(), SIGNAL(windowCloseRequested()), this, SLOT(closeTab()));
    connect(page(), SIGNAL(downloadRequested(const QNetworkRequest &)), this, SLOT(downloadRequested(const QNetworkRequest &)));

    connect(mApp->networkManager(), SIGNAL(finishLoading(bool)), this, SLOT(loadFinished(bool)));
    connect(mApp->networkManager(), SIGNAL(wantsFocus(QUrl)), this, SLOT(getFocus(QUrl)));

    connect(p_QupZilla, SIGNAL(setWebViewMouseTracking(bool)), this, SLOT(trackMouse(bool)));

    // Tracking mouse also on tabs created in fullscreen
    trackMouse(p_QupZilla->isFullScreen());

    // Zoom levels same as in firefox
    m_zoomLevels << 30 << 50 << 67 << 80 << 90 << 100 << 110 << 120 << 133 << 150 << 170 << 200 << 240 << 300;
    // Set default zoom
    m_currentZoom = mApp->defaultZoom();
    applyZoom();

    qApp->installEventFilter(this);
}

void WebView::slotIconChanged()
{
    m_siteIcon = icon();

    if (url().scheme() == "file" || title().contains(tr("Failed loading page"))) {
        return;
    }

    mApp->iconProvider()->saveIcon(this);
}

void WebView::copyText()
{
    if (!selectedText().isEmpty()) {
        QApplication::clipboard()->setText(selectedText());
    }
}

WebPage* WebView::webPage() const
{
    return m_page;
}

void WebView::back()
{
    if (page())  {
        emit ipChanged(m_currentIp);
        p_QupZilla->navigationBar()->goBack();
    }
}

void WebView::forward()
{
    if (page()) {
        emit ipChanged(m_currentIp);
        p_QupZilla->navigationBar()->goForward();
    }
}

void WebView::slotReload()
{
    if (page()) {
        emit ipChanged(m_currentIp);
        page()->triggerAction(QWebPage::Reload);
    }
}

WebTab* WebView::webTab() const
{
    return m_webTab;
}

bool WebView::isCurrent()
{
    if (!tabWidget()) {
        return false;
    }
    WebTab* webTab = qobject_cast<WebTab*>(tabWidget()->widget(tabWidget()->currentIndex()));
    if (!webTab) {
        return false;
    }

    return (webTab->view() == this);
}

void WebView::urlChanged(const QUrl &url)
{
    if (isCurrent()) {
        p_QupZilla->navigationBar()->refreshHistory();
    }

    if (m_lastUrl != url) {
        emit changed();
    }

    emit showUrl(url);
}

void WebView::linkClicked(const QUrl &url)
{
    load(url);
}

void WebView::setProgress(int prog)
{
    m_progress = prog;

    if (prog > 60) {
        checkRss();
    }

    if (isCurrent()) {
        p_QupZilla->ipLabel()->hide();
        p_QupZilla->progressBar()->setVisible(true);
        p_QupZilla->progressBar()->setValue(m_progress);
        p_QupZilla->navigationBar()->showStopButton();
    }
}

void WebView::loadStarted()
{
    m_progress = 0;
    m_isLoading = true;
    m_rssChecked = false;
    emit rssChanged(false);

    animationLoading(tabIndex(), true);

    if (title().isNull()) {
        tabWidget()->setTabText(tabIndex(), tr("Loading..."));
    }

    m_currentIp.clear();

//    if (m_loadingTimer)
//        delete m_loadingTimer;
//    m_loadingTimer = new QTimer();
//    connect(m_loadingTimer, SIGNAL(timeout()), this, SLOT(stopAnimation()));
//    m_loadingTimer->start(1000*20); //20 seconds timeout to automatically "stop" loading animation
}

void WebView::loadFinished(bool state)
{
    Q_UNUSED(state);

    if (mApp->isClosing() || p_QupZilla->isClosing()) {
        return;
    }

    if (animationLoading(tabIndex(), false)->movie()) {
        animationLoading(tabIndex(), false)->movie()->stop();
    }

    m_isLoading = false;

    if (m_lastUrl != url()) {
        mApp->history()->addHistoryEntry(this);
    }

    emit showUrl(url());

    iconChanged();
    m_lastUrl = url();

    //Icon is sometimes not available at the moment of finished loading
    if (icon().isNull()) {
        QTimer::singleShot(1000, this, SLOT(iconChanged()));
    }

    titleChanged();
    mApp->autoFill()->completePage(this);
    QHostInfo::lookupHost(url().host(), this, SLOT(setIp(QHostInfo)));

    if (isCurrent()) {
        p_QupZilla->progressBar()->setVisible(false);
        p_QupZilla->navigationBar()->showReloadButton();
        p_QupZilla->ipLabel()->show();
    }

    emit urlChanged(url());
}

QLabel* WebView::animationLoading(int index, bool addMovie)
{
    if (-1 == index) {
        return 0;
    }

    QLabel* loadingAnimation = qobject_cast<QLabel*>(tabWidget()->getTabBar()->tabButton(index, QTabBar::LeftSide));
    if (!loadingAnimation) {
        loadingAnimation = new QLabel();
//        loadingAnimation->setStyleSheet("margin: 0px; padding: 0px; width: 16px; height: 16px;");
    }
    if (addMovie && !loadingAnimation->movie()) {
        QMovie* movie = new QMovie(":icons/other/progress.gif", QByteArray(), loadingAnimation);
        movie->setSpeed(70);
        loadingAnimation->setMovie(movie);
        movie->start();
    }
    else if (loadingAnimation->movie()) {
        loadingAnimation->movie()->stop();
    }

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
    }

    iconChanged();
}

void WebView::setIp(const QHostInfo &info)
{
    if (info.addresses().isEmpty()) {
        return;
    }
    m_currentIp = info.hostName() + " (" + info.addresses().at(0).toString() + ")";

    if (isCurrent()) {
        emit ipChanged(m_currentIp);
    }
}

void WebView::titleChanged()
{
    QString title_ = title();
    QString title2 = title_;
    tabWidget()->setTabToolTip(tabIndex(), title2);

    title2 += " - QupZilla";
    if (isCurrent()) {
        p_QupZilla->setWindowTitle(title2);
    }

    tabWidget()->setTabText(tabIndex(), title_);
}

void WebView::iconChanged()
{
    if (mApp->isClosing() || p_QupZilla->isClosing()) {
        return;
    }

//    if (isCurrent())
    emit siteIconChanged();

    if (m_isLoading) {
        return;
    }

    QIcon icon_ = siteIcon();
    if (!icon_.isNull()) {
        animationLoading(tabIndex(), false)->setPixmap(icon_.pixmap(16, 16));
    }
    else {
        animationLoading(tabIndex(), false)->setPixmap(IconProvider::fromTheme("text-plain").pixmap(16, 16));
    }
}

QIcon WebView::siteIcon()
{
    if (url().scheme() == "qupzilla") {
        return QIcon(":icons/qupzilla.png");
    }

    if (!icon().isNull()) {
        return icon();
    }
    if (!m_siteIcon.isNull()) {
        return m_siteIcon;
    }

    return _iconForUrl(url());
}

void WebView::linkHovered(const QString &link, const QString &title, const QString &content)
{
    Q_UNUSED(title)
    Q_UNUSED(content)

    if (isCurrent()) {
        if (link != "") {
            p_QupZilla->statusBarMessage()->showMessage(link);
        }
        else {
            p_QupZilla->statusBarMessage()->clearMessage();
        }
    }
    m_hoveredLink = link;
}

TabWidget* WebView::tabWidget() const
{
    QObject* widget = this->parent();
    while (widget) {
        if (TabWidget* tw = qobject_cast<TabWidget*>(widget)) {
            return tw;
        }
        widget = widget->parent();
    }
    return 0;
}

int WebView::tabIndex() const
{
    TabWidget* tabWid = tabWidget();
    if (!tabWid) {
        return -1;
    }

    int i = 0;
    while (qobject_cast<WebTab*>(tabWid->widget(i))->view() != this) {
        i++;
    }
    return i;
}

QUrl WebView::guessUrlFromString(const QString &string)
{
    QString trimmedString = string.trimmed();

    // Check the most common case of a valid url with scheme and host first
    QUrl url = QUrl::fromEncoded(trimmedString.toUtf8(), QUrl::TolerantMode);
    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty()) {
        return url;
    }

    // Absolute files that exists
    if (QDir::isAbsolutePath(trimmedString) && QFile::exists(trimmedString)) {
        return QUrl::fromLocalFile(trimmedString);
    }

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

    if (url.isValid()) {
        return url;
    }

    return QUrl();
}

void WebView::checkRss()
{
    if (m_rssChecked) {
        return;
    }

    m_rssChecked = true;
    QWebFrame* frame = page()->mainFrame();
    QWebElementCollection links = frame->findAllElements("link[type=\"application/rss+xml\"]");

    m_hasRss = links.count() != 0;
    emit rssChanged(m_hasRss);
}

void WebView::contextMenuEvent(QContextMenuEvent* event)
{
    m_menu->clear();

    QWebFrame* frameAtPos = page()->frameAt(event->pos());
    QWebHitTestResult r = page()->mainFrame()->hitTestContent(event->pos());

    if (!r.linkUrl().isEmpty() && r.linkUrl().scheme() != "javascript") {
        if (page()->selectedText() == r.linkText()) {
            findText("");
        }
        m_menu->addAction(QIcon(":/icons/menu/popup.png"), tr("Open link in new &tab"), this, SLOT(openUrlInNewTab()))->setData(r.linkUrl());
        m_menu->addAction(tr("Open link in new &window"), this, SLOT(openUrlInNewWindow()))->setData(r.linkUrl());
        m_menu->addSeparator();
        m_menu->addAction(IconProvider::fromTheme("user-bookmarks"), tr("B&ookmark link"), this, SLOT(bookmarkLink()))->setData(r.linkUrl());
        m_menu->addAction(QIcon::fromTheme("document-save"), tr("&Save link as..."), this, SLOT(downloadLinkToDisk()))->setData(r.linkUrl());
        m_menu->addAction(tr("Send link..."), this, SLOT(sendLinkByMail()))->setData(r.linkUrl());
        m_menu->addAction(QIcon::fromTheme("edit-copy"), tr("&Copy link address"), this, SLOT(copyLinkToClipboard()))->setData(r.linkUrl());
        m_menu->addSeparator();
        if (!selectedText().isEmpty()) {
            m_menu->addAction(pageAction(QWebPage::Copy));
        }
    }

    if (!r.imageUrl().isEmpty()) {
        if (!m_menu->isEmpty()) {
            m_menu->addSeparator();
        }
        m_menu->addAction(tr("Show i&mage"), this, SLOT(showImage()))->setData(r.imageUrl());
        m_menu->addAction(tr("Copy im&age"), this, SLOT(copyImageToClipboard()))->setData(r.imageUrl());
        m_menu->addAction(QIcon::fromTheme("edit-copy"), tr("Copy image ad&dress"), this, SLOT(copyLinkToClipboard()))->setData(r.imageUrl());
        m_menu->addSeparator();
        m_menu->addAction(QIcon::fromTheme("document-save"), tr("&Save image as..."), this, SLOT(downloadImageToDisk()))->setData(r.imageUrl());
        m_menu->addAction(tr("Send image..."), this, SLOT(sendLinkByMail()))->setData(r.linkUrl());
        m_menu->addSeparator();
        //menu->addAction(tr("Block image"), this, SLOT(blockImage()))->setData(r.imageUrl().toString());
        if (!selectedText().isEmpty()) {
            m_menu->addAction(pageAction(QWebPage::Copy));
        }
    }

    QWebElement element = r.element();
    if (!element.isNull() && (element.tagName().toLower() == "input" || element.tagName().toLower() == "textarea" ||
                              element.tagName().toLower() == "video" || element.tagName().toLower() == "audio")) {
        if (m_menu->isEmpty()) {
            page()->createStandardContextMenu()->popup(QCursor::pos());
            return;
        }
    }

    if (m_menu->isEmpty()) {
        QAction* action = m_menu->addAction(tr("&Back"), this, SLOT(back()));
        action->setIcon(IconProvider::standardIcon(QStyle::SP_ArrowBack));
        action->setEnabled(history()->canGoBack());

        action = m_menu->addAction(tr("&Forward"), this, SLOT(forward()));
        action->setIcon(IconProvider::standardIcon(QStyle::SP_ArrowForward));
        action->setEnabled(history()->canGoForward());

        m_menu->addAction(IconProvider::standardIcon(QStyle::SP_BrowserReload), tr("&Reload"), this, SLOT(slotReload()));
        action = m_menu->addAction(IconProvider::standardIcon(QStyle::SP_BrowserStop), tr("S&top"), this, SLOT(stop()));
        action->setEnabled(isLoading());
        m_menu->addSeparator();

        if (frameAtPos && page()->mainFrame() != frameAtPos) {
            m_clickedFrame = frameAtPos;
            QMenu* menu = new QMenu(tr("This frame"));
            menu->addAction(tr("Show &only this frame"), this, SLOT(loadClickedFrame()));
            menu->addAction(QIcon(":/icons/menu/popup.png"), tr("Show this frame in new &tab"), this, SLOT(loadClickedFrameInNewTab()));
            menu->addSeparator();
            menu->addAction(IconProvider::standardIcon(QStyle::SP_BrowserReload), tr("&Reload"), this, SLOT(reloadClickedFrame()));
            menu->addAction(QIcon::fromTheme("document-print"), tr("Print frame"), this, SLOT(printClickedFrame()));
            menu->addSeparator();
            menu->addAction(QIcon::fromTheme("zoom-in"), tr("Zoom &in"), this, SLOT(clickedFrameZoomIn()));
            menu->addAction(QIcon::fromTheme("zoom-out"), tr("&Zoom out"), this, SLOT(clickedFrameZoomOut()));
            menu->addAction(QIcon::fromTheme("zoom-original"), tr("Reset"), this, SLOT(clickedFrameZoomReset()));
            menu->addSeparator();
            menu->addAction(QIcon::fromTheme("text-html"), tr("Show so&urce of frame"), this, SLOT(showClickedFrameSource()));

            m_menu->addMenu(menu);
        }

        m_menu->addSeparator();
        m_menu->addAction(IconProvider::fromTheme("user-bookmarks"), tr("Book&mark page"), this, SLOT(bookmarkLink()));
        m_menu->addAction(QIcon::fromTheme("document-save"), tr("&Save page as..."), this, SLOT(downloadLinkToDisk()))->setData(url());
        m_menu->addAction(tr("Send page..."), this, SLOT(sendLinkByMail()))->setData(url());
        m_menu->addSeparator();
        m_menu->addAction(QIcon::fromTheme("edit-select-all"), tr("Select &all"), this, SLOT(selectAll()));
        if (!selectedText().isEmpty()) {
            m_menu->addAction(pageAction(QWebPage::Copy));
        }

        m_menu->addSeparator();
        m_menu->addAction(QIcon::fromTheme("text-html"), tr("Show so&urce code"), this, SLOT(showSource()));
        m_menu->addAction(QIcon::fromTheme("dialog-information"), tr("Show info ab&out site"), this, SLOT(showSiteInfo()))->setData(url());
        m_menu->addAction(tr("Show Web &Inspector"), this, SLOT(showInspector()));
    }

    mApp->plugins()->populateWebViewMenu(m_menu, this, r);

    if (!selectedText().isEmpty()) {
        m_menu->addSeparator();
        QString selectedText = page()->selectedText();
        selectedText.truncate(20);

        SearchEngine engine = mApp->searchEnginesManager()->activeEngine();
        m_menu->addAction(engine.icon, tr("Search \"%1 ..\" with %2").arg(selectedText, engine.name), this, SLOT(searchSelectedText()));
    }

#if (QTWEBKIT_VERSION >= QTWEBKIT_VERSION_CHECK(2, 2, 0))
//    still bugged in 4.8 RC (it shows selection of webkit's internal source, not html from page)
//    it may or may not be bug, but this implementation is useless for us
//
//    if (!selectedHtml().isEmpty())
//        menu->addAction(tr("Show source of selection"), this, SLOT(showSourceOfSelection()));
#endif

    if (!m_menu->isEmpty()) {
        //Prevent choosing first option with double rightclick
        QPoint pos = QCursor::pos();
        QPoint p(pos.x(), pos.y() + 1);
        m_menu->popup(p);
        return;
    }

    QWebView::contextMenuEvent(event);
}

void WebView::stop()
{
    if (page()) {
        emit ipChanged(m_currentIp);
        page()->triggerAction(QWebPage::Stop);
        loadFinished(true);

        if (m_locationBar->text().isEmpty()) {
            setFocus();
            emit urlChanged(url());
        }
    }
}

void WebView::addNotification(QWidget* notif)
{
    emit showNotification(notif);
}

void WebView::openUrlInNewTab()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        tabWidget()->addView(action->data().toUrl(), tr("New tab"), TabWidget::NewBackgroundTab);
    }
}

void WebView::openUrlInNewWindow()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        mApp->makeNewWindow(false, action->data().toString());
    }
}

void WebView::sendLinkByMail()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        QDesktopServices::openUrl(QUrl("mailto:?body=" + action->data().toString()));
    }
}

void WebView::copyLinkToClipboard()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        QApplication::clipboard()->setText(action->data().toString());
    }
}

void WebView::searchSelectedText()
{
    SearchEngine engine = mApp->searchEnginesManager()->activeEngine();
    load(engine.url.replace("%s", selectedText()));
}

void WebView::selectAll()
{
    triggerPageAction(QWebPage::SelectAll);
}

void WebView::downloadImageToDisk()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        DownloadManager* dManager = mApp->downManager();
        QNetworkRequest request(action->data().toUrl());
        dManager->download(request, m_page, false);
    }
}

void WebView::copyImageToClipboard()
{
    triggerPageAction(QWebPage::CopyImageToClipboard);
}

void WebView::showImage()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        load(QUrl(action->data().toString()));
    }
}

void WebView::showSource()
{
    p_QupZilla->showSource();
}

void WebView::showSourceOfSelection()
{
#if (QTWEBKIT_VERSION >= QTWEBKIT_VERSION_CHECK(2, 2, 0))
    p_QupZilla->showSource(page()->mainFrame(), selectedHtml());
#endif
}

void WebView::downloadLinkToDisk()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        QNetworkRequest request(action->data().toUrl());
        DownloadManager* dManager = mApp->downManager();
        dManager->download(request, m_page, false);
    }
}

void WebView::downloadRequested(const QNetworkRequest &request)
{
    DownloadManager* dManager = mApp->downManager();
    dManager->download(request, m_page);
}

void WebView::bookmarkLink()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        if (action->data().isNull()) {
            p_QupZilla->bookmarkPage();
        }
        else {
            p_QupZilla->addBookmark(action->data().toUrl(), title(), siteIcon());
        }
    }
}

void WebView::showInspector()
{
    p_QupZilla->showWebInspector();
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

    if (i < m_zoomLevels.count() - 1) {
        m_currentZoom = m_zoomLevels[i + 1];
    }
    applyZoom();
}

void WebView::zoomOut()
{
    int i = m_zoomLevels.indexOf(m_currentZoom);

    if (i > 0) {
        m_currentZoom = m_zoomLevels[i - 1];
    }
    applyZoom();
}

void WebView::zoomReset()
{
    m_currentZoom = 100;
    applyZoom();
}

void WebView::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        int numDegrees = event->delta() / 8;
        int numSteps = numDegrees / 15;
        if (numSteps == 1) {
            zoomIn();
        }
        else {
            zoomOut();
        }
        event->accept();
        return;
    }
    if (m_mouseWheelEnabled) {
        QWebView::wheelEvent(event);
    }
}

void WebView::getFocus(const QUrl &urla)
{
    if (urla == url()) {
        tabWidget()->setCurrentWidget(this);
    }
}

void WebView::closeTab()
{
    if (m_wantsClose) {
        emit wantsCloseTab(tabIndex());
    }
    else {
        m_wantsClose = true;
        QTimer::singleShot(100, this, SLOT(closeTab()));
    }
}

void WebView::load(const QUrl &url)
{
    if (url.scheme() == "javascript") {
        page()->mainFrame()->evaluateJavaScript(url.toString());
        return;
    }

    if (url.scheme() == "data" || url.scheme() == "qrc") {
        QWebView::load(url);
        emit urlChanged(url);
        m_aboutToLoadUrl = url;
        return;
    }

    if (isUrlValid(url)) {
        QWebView::load(url);
        emit urlChanged(url);
        m_aboutToLoadUrl = url;
        return;
    }

    if (QFile::exists(url.toLocalFile())) {
        QWebView::load(url);
        emit urlChanged(url);
    }
    else {
        SearchEngine engine = mApp->searchEnginesManager()->activeEngine();
        QString urlString = engine.url.replace("%s", url.toString());
        QWebView::load(QUrl(urlString));
        emit urlChanged(url);
    }
}

QUrl WebView::url() const
{
    return QWebView::url();
    QUrl ur = QWebView::url();
    if (ur.isEmpty() && !m_aboutToLoadUrl.isEmpty()) {
        return m_aboutToLoadUrl;
    }
    return ur;
}

QString WebView::title() const
{
    QString title = QWebView::title();
    if (title.isEmpty()) {
        title = url().toString(QUrl::RemoveFragment);
    }
    if (title.isEmpty()) {
        return tr("No Named Page");
    }

    return title;
}

void WebView::reload()
{
    if (QWebView::url().isEmpty() && !m_aboutToLoadUrl.isEmpty()) {
//        qDebug() << "loading about to load";
        load(m_aboutToLoadUrl);
        return;
    }
    QWebView::reload();
}

bool WebView::isUrlValid(const QUrl &url)
{
    if (url.scheme() == "qupzilla") {
        return true;
    }

    if (url.isValid() && !url.host().isEmpty() && !url.scheme().isEmpty()) {
        return true;
    }
    return false;
}

// ClickedFrame slots

void WebView::loadClickedFrame()
{
    load(m_clickedFrame->url());
}

void WebView::loadClickedFrameInNewTab()
{
    p_QupZilla->tabWidget()->addView(m_clickedFrame->url());
}

void WebView::reloadClickedFrame()
{
    m_clickedFrame->load(m_clickedFrame->url());
}

void WebView::printClickedFrame()
{
    p_QupZilla->printPage(m_clickedFrame);
}

void WebView::clickedFrameZoomIn()
{
    qreal zFactor = m_clickedFrame->zoomFactor() + 0.1;
    if (zFactor > 2.5) {
        zFactor = 2.5;
    }

    m_clickedFrame->setZoomFactor(zFactor);
}

void WebView::clickedFrameZoomOut()
{
    qreal zFactor = m_clickedFrame->zoomFactor() - 0.1;
    if (zFactor < 0.5) {
        zFactor = 0.5;
    }

    m_clickedFrame->setZoomFactor(zFactor);
}

void WebView::clickedFrameZoomReset()
{
    m_clickedFrame->setZoomFactor(zoomFactor());
}

void WebView::showClickedFrameSource()
{
    p_QupZilla->showSource(m_clickedFrame);
}

void WebView::mousePressEvent(QMouseEvent* event)
{
    switch (event->button()) {
    case Qt::XButton1:
        back();
        break;
    case Qt::XButton2:
        forward();
        break;
    case Qt::MiddleButton:
        if (isUrlValid(QUrl(m_hoveredLink))) {
            tabWidget()->addView(QUrl::fromEncoded(m_hoveredLink.toUtf8()), tr("New tab"), TabWidget::NewBackgroundTab);
            event->accept();
            return;
        }
#ifdef Q_WS_WIN
        else {
            QWebView::mouseDoubleClickEvent(event);
            return;
        }
#endif
        break;
    case Qt::LeftButton:
        if (event->modifiers() == Qt::ControlModifier && isUrlValid(QUrl(m_hoveredLink))) {
            tabWidget()->addView(QUrl::fromEncoded(m_hoveredLink.toUtf8()), tr("New tab"), TabWidget::NewBackgroundTab);
            return;
        }
    default:
        break;
    }

    QWebView::mousePressEvent(event);
}

void WebView::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
    case Qt::Key_C:
        if (event->modifiers() == Qt::ControlModifier) {
            copyText();
            return;
        }
        break;

    case Qt::Key_A:
        if (event->modifiers() == Qt::ControlModifier) {
            selectAll();
            return;
        }
        break;

    default:
        break;
    }

    QWebView::keyPressEvent(event);
}

void WebView::resizeEvent(QResizeEvent* event)
{
    QWebView::resizeEvent(event);
    emit viewportResized(m_page->viewportSize());
}

void WebView::mouseReleaseEvent(QMouseEvent* event)
{
    //Workaround for crash in mouseReleaseEvent when closing tab from javascript :/
    if (!m_wantsClose) {
        QWebView::mouseReleaseEvent(event);
    }
}

void WebView::mouseMoveEvent(QMouseEvent* event)
{
    if (m_mouseTrack) {
        if (m_navigationVisible) {
            m_navigationVisible = false;
            p_QupZilla->showNavigationWithFullscreen();
        }
        else if (event->y() < 5) {
            m_navigationVisible = true;
            p_QupZilla->showNavigationWithFullscreen();
        }
    }
    QWebView::mouseMoveEvent(event);
}

///
// This function was taken and modified from QTestBrowser to fix bug #33 with flightradar24.com
// You can find original source and copyright here:
// http://gitorious.org/+qtwebkit-developers/webkit/qtwebkit/blobs/qtwebkit-2.2/Tools/QtTestBrowser/launcherwindow.cpp
///
bool WebView::eventFilter(QObject* obj, QEvent* event)
{
    if (obj != this) {
        return false;
    }

    if (event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::MouseButtonRelease ||
            event->type() == QEvent::MouseButtonDblClick ||
            event->type() == QEvent::MouseMove) {

        QMouseEvent* ev = static_cast<QMouseEvent*>(event);
        if (ev->type() == QEvent::MouseMove
                && !(ev->buttons() & Qt::LeftButton)) {
            return false;
        }

        QTouchEvent::TouchPoint touchPoint;
        touchPoint.setState(Qt::TouchPointMoved);
        if ((ev->type() == QEvent::MouseButtonPress
                || ev->type() == QEvent::MouseButtonDblClick)) {
            touchPoint.setState(Qt::TouchPointPressed);
        }
        else if (ev->type() == QEvent::MouseButtonRelease) {
            touchPoint.setState(Qt::TouchPointReleased);
        }

        touchPoint.setId(0);
        touchPoint.setScreenPos(ev->globalPos());
        touchPoint.setPos(ev->pos());
        touchPoint.setPressure(1);

        // If the point already exists, update it. Otherwise create it.
        if (m_touchPoints.size() > 0 && !m_touchPoints[0].id()) {
            m_touchPoints[0] = touchPoint;
        }
        else if (m_touchPoints.size() > 1 && !m_touchPoints[1].id()) {
            m_touchPoints[1] = touchPoint;
        }
        else {
            m_touchPoints.append(touchPoint);
        }

        if (!m_touchPoints.isEmpty()) {
            QEvent::Type type = QEvent::TouchUpdate;
            if (m_touchPoints.size() == 1) {
                if (m_touchPoints[0].state() == Qt::TouchPointReleased) {
                    type = QEvent::TouchEnd;
                }
                else if (m_touchPoints[0].state() == Qt::TouchPointPressed) {
                    type = QEvent::TouchBegin;
                }
            }

            QTouchEvent touchEv(type);
            touchEv.setTouchPoints(m_touchPoints);
            QCoreApplication::sendEvent(page(), &touchEv);

            // After sending the event, remove all touchpoints that were released
            if (m_touchPoints[0].state() == Qt::TouchPointReleased) {
                m_touchPoints.removeAt(0);
            }
            if (m_touchPoints.size() > 1 && m_touchPoints[1].state() == Qt::TouchPointReleased) {
                m_touchPoints.removeAt(1);
            }
        }

        return false;
    }

    return QWebView::eventFilter(obj, event);
}

WebView::~WebView()
{
}
