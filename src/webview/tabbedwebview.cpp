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
#include "tabbedwebview.h"
#include "qupzilla.h"
#include "webpage.h"
#include "tabwidget.h"
#include "networkmanager.h"
#include "mainapplication.h"
#include "tabbar.h"
#include "pluginproxy.h"
#include "webtab.h"
#include "statusbarmessage.h"
#include "progressbar.h"
#include "navigationbar.h"
#include "iconprovider.h"
#include "searchenginesmanager.h"

TabbedWebView::TabbedWebView(QupZilla* mainClass, WebTab* webTab)
    : WebView(webTab)
    , p_QupZilla(mainClass)
    , m_tabWidget(p_QupZilla->tabWidget())
    , m_page(0)
    , m_webTab(webTab)
    , m_menu(new QMenu(this))
    , m_clickedFrame(0)
    , m_mouseTrack(false)
    , m_navigationVisible(false)
    , m_hasRss(false)
    , m_rssChecked(false)
{
    connect(this, SIGNAL(loadStarted()), this, SLOT(slotLoadStarted()));
    connect(this, SIGNAL(loadProgress(int)), this, SLOT(loadingProgress(int)));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(slotLoadFinished()));

    connect(this, SIGNAL(urlChanged(QUrl)), this, SLOT(urlChanged(QUrl)));
    connect(this, SIGNAL(titleChanged(QString)), this, SLOT(titleChanged()));
    connect(this, SIGNAL(iconChanged()), this, SLOT(slotIconChanged()));

    connect(this, SIGNAL(statusBarMessage(QString)), p_QupZilla->statusBar(), SLOT(showMessage(QString)));

    connect(mApp->networkManager(), SIGNAL(wantsFocus(QUrl)), this, SLOT(getFocus(QUrl)));

    connect(p_QupZilla, SIGNAL(setWebViewMouseTracking(bool)), this, SLOT(trackMouse(bool)));

    // Tracking mouse also on tabs created in fullscreen
    trackMouse(p_QupZilla->isFullScreen());
}

void TabbedWebView::setWebPage(WebPage* page)
{
    if (m_page == page) {
        return;
    }

    if (m_page) {
        delete m_page;
        m_page = 0;
    }

    m_page = page;
    m_page->setWebView(this);
    m_page->setParent(this);
    setPage(m_page);

    connect(m_page, SIGNAL(linkHovered(QString, QString, QString)), this, SLOT(linkHovered(QString, QString, QString)));
    connect(m_page, SIGNAL(windowCloseRequested()), this, SLOT(closeView()));

    // Set default zoom
    setZoom(mApp->defaultZoom());
}

void TabbedWebView::slotIconChanged()
{
    if (url().scheme() == "file" || url().scheme() == "qupzilla" || title().contains(tr("Failed loading page"))) {
        return;
    }

    mApp->iconProvider()->saveIcon(this);

    showIcon();
}

WebPage* TabbedWebView::webPage() const
{
    return m_page;
}

WebTab* TabbedWebView::webTab() const
{
    return m_webTab;
}

QString TabbedWebView::getIp() const
{
    return m_currentIp;
}

bool TabbedWebView::isCurrent()
{
    WebTab* webTab = qobject_cast<WebTab*>(m_tabWidget->widget(m_tabWidget->currentIndex()));
    if (!webTab) {
        return false;
    }

    return (webTab->view() == this);
}

void TabbedWebView::urlChanged(const QUrl &url)
{
    if (isCurrent()) {
        p_QupZilla->navigationBar()->refreshHistory();
    }

    if (lastUrl() != url) {
        emit changed();
    }
}

void TabbedWebView::loadingProgress(int prog)
{
    if (prog > 60) {
        checkRss();
    }

    if (isCurrent()) {
        p_QupZilla->updateLoadingActions();
    }
}

void TabbedWebView::slotLoadStarted()
{
    m_rssChecked = false;
    emit rssChanged(false);

    animationLoading(tabIndex(), true);

    if (title().isNull()) {
        m_tabWidget->setTabText(tabIndex(), tr("Loading..."));
    }

    m_currentIp.clear();
}

void TabbedWebView::slotLoadFinished()
{
    QMovie* mov = animationLoading(tabIndex(), false)->movie();
    if (mov) {
        mov->stop();
    }

    showIcon();
    QHostInfo::lookupHost(url().host(), this, SLOT(setIp(QHostInfo)));

    if (isCurrent()) {
        p_QupZilla->updateLoadingActions();
    }
}

QLabel* TabbedWebView::animationLoading(int index, bool addMovie)
{
    if (index == -1) {
        return 0;
    }

    QLabel* loadingAnimation = qobject_cast<QLabel*>(m_tabWidget->getTabBar()->tabButton(index, QTabBar::LeftSide));
    if (!loadingAnimation) {
        loadingAnimation = new QLabel();
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

    m_tabWidget->getTabBar()->setTabButton(index, QTabBar::LeftSide, 0);
    m_tabWidget->getTabBar()->setTabButton(index, QTabBar::LeftSide, loadingAnimation);
    return loadingAnimation;
}

void TabbedWebView::stopAnimation()
{
    QMovie* mov = animationLoading(tabIndex(), false)->movie();
    if (mov) {
        mov->stop();
    }

    showIcon();
}

void TabbedWebView::setIp(const QHostInfo &info)
{
    if (info.addresses().isEmpty()) {
        return;
    }

    m_currentIp = info.hostName() + " (" + info.addresses().at(0).toString() + ")";

    if (isCurrent()) {
        emit ipChanged(m_currentIp);
    }
}

void TabbedWebView::titleChanged()
{
    QString t = title();
    m_tabWidget->setTabToolTip(tabIndex(), t);

    if (isCurrent()) {
        p_QupZilla->setWindowTitle(tr("%1 - QupZilla").arg(t));
    }

    m_tabWidget->setTabText(tabIndex(), t);
}

void TabbedWebView::showIcon()
{
    if (isLoading()) {
        return;
    }

    QIcon icon_ = icon();
    if (!icon_.isNull()) {
        animationLoading(tabIndex(), false)->setPixmap(icon_.pixmap(16, 16));
    }
    else {
        animationLoading(tabIndex(), false)->setPixmap(IconProvider::fromTheme("text-plain").pixmap(16, 16));
    }
}

void TabbedWebView::linkHovered(const QString &link, const QString &title, const QString &content)
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

// FIXME: Don't do this magic to get index of tab.
// Implement setTabIndex() and call it from TabWidget (when creating and also from
// tabMoved slot)
int TabbedWebView::tabIndex() const
{
    int i = 0;
    while (WebTab* wTab = qobject_cast<WebTab*>(m_tabWidget->widget(i))) {
        if (wTab && wTab->view() == this) {
            break;
        }
        i++;
    }
    return i;
}

QWidget* TabbedWebView::overlayForJsAlert()
{
    return m_webTab;
}

void TabbedWebView::closeView()
{
    emit wantsCloseTab(tabIndex());
}

void TabbedWebView::checkRss()
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

void TabbedWebView::contextMenuEvent(QContextMenuEvent* event)
{
    m_menu->clear();
    m_clickedFrame = 0;

    QWebFrame* frameAtPos = page()->frameAt(event->pos());
    QWebHitTestResult r = page()->mainFrame()->hitTestContent(event->pos());

    if (!r.linkUrl().isEmpty() && r.linkUrl().scheme() != "javascript") {
        if (page()->selectedText() == r.linkText()) {
            findText("");
        }
        m_menu->addAction(QIcon(":/icons/menu/popup.png"), tr("Open link in new &tab"), this, SLOT(openUrlInNewTab()))->setData(r.linkUrl());
        m_menu->addAction(QIcon::fromTheme("window-new"), tr("Open link in new &window"), this, SLOT(openUrlInNewWindow()))->setData(r.linkUrl());
        m_menu->addSeparator();
        m_menu->addAction(IconProvider::fromTheme("user-bookmarks"), tr("B&ookmark link"), this, SLOT(bookmarkLink()))->setData(r.linkUrl());
        m_menu->addAction(QIcon::fromTheme("document-save"), tr("&Save link as..."), this, SLOT(downloadLinkToDisk()))->setData(r.linkUrl());
        m_menu->addAction(QIcon::fromTheme("mail-message-new"), tr("Send link..."), this, SLOT(sendLinkByMail()))->setData(r.linkUrl());
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
        m_menu->addAction(tr("Show i&mage"), this, SLOT(openActionUrl()))->setData(r.imageUrl());
        m_menu->addAction(tr("Copy im&age"), this, SLOT(copyImageToClipboard()))->setData(r.imageUrl());
        m_menu->addAction(QIcon::fromTheme("edit-copy"), tr("Copy image ad&dress"), this, SLOT(copyLinkToClipboard()))->setData(r.imageUrl());
        m_menu->addSeparator();
        m_menu->addAction(QIcon::fromTheme("document-save"), tr("&Save image as..."), this, SLOT(downloadLinkToDisk()))->setData(r.imageUrl());
        m_menu->addAction(QIcon::fromTheme("mail-message-new"), tr("Send image..."), this, SLOT(sendLinkByMail()))->setData(r.imageUrl());
        m_menu->addSeparator();
        //menu->addAction(tr("Block image"), this, SLOT(blockImage()))->setData(r.imageUrl().toString());
        if (!selectedText().isEmpty()) {
            m_menu->addAction(pageAction(QWebPage::Copy));
        }
    }

    QWebElement element = r.element();
    if (!element.isNull() && (element.tagName().toLower() == "input" || element.tagName().toLower() == "textarea")) {
        if (m_menu->isEmpty()) {
            page()->createStandardContextMenu()->popup(QCursor::pos());
            return;
        }
    }

    if (isMediaElement(element)) {
        if (m_menu->isEmpty()) {
            createMediaContextMenu(r)->popup(QCursor::pos());
            return;
        }
    }

    if (m_menu->isEmpty() && selectedText().isEmpty()) {
        QAction* action = m_menu->addAction(tr("&Back"), this, SLOT(back()));
        action->setIcon(IconProvider::standardIcon(QStyle::SP_ArrowBack));
        action->setEnabled(history()->canGoBack());

        action = m_menu->addAction(tr("&Forward"), this, SLOT(forward()));
        action->setIcon(IconProvider::standardIcon(QStyle::SP_ArrowForward));
        action->setEnabled(history()->canGoForward());

        m_menu->addAction(IconProvider::standardIcon(QStyle::SP_BrowserReload), tr("&Reload"), this, SLOT(reload()));
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
        m_menu->addAction(QIcon::fromTheme("edit-copy"), tr("&Copy page link"), this, SLOT(copyLinkToClipboard()))->setData(url());
        m_menu->addAction(QIcon::fromTheme("mail-message-new"), tr("Send page link..."), this, SLOT(sendLinkByMail()))->setData(url());
        m_menu->addAction(QIcon::fromTheme("document-print"), tr("&Print page"), this, SLOT(printPage()));
        m_menu->addSeparator();
        m_menu->addAction(QIcon::fromTheme("edit-select-all"), tr("Select &all"), this, SLOT(selectAll()));
        m_menu->addSeparator();
        if (url().scheme() == "http" || url().scheme() == "https") {
//             bool result = validateConfirm(tr("Do you want to upload this page to an online source code validator?"));
//                 if (result)
            m_menu->addAction(tr("Validate page"), this, SLOT(openUrlInNewTab()))->setData("http://validator.w3.org/check?uri=" + url().toString());
        }

        m_menu->addAction(QIcon::fromTheme("text-html"), tr("Show so&urce code"), this, SLOT(showSource()));
        m_menu->addAction(tr("Show Web &Inspector"), this, SLOT(showInspector()));
        m_menu->addSeparator();
        m_menu->addAction(QIcon::fromTheme("dialog-information"), tr("Show info ab&out site"), this, SLOT(showSiteInfo()));
    }

    mApp->plugins()->populateWebViewMenu(m_menu, this, r);

    if (!selectedText().isEmpty()) {
        QString selectedText = page()->selectedText();

        m_menu->addAction(pageAction(QWebPage::Copy));
        m_menu->addAction(QIcon::fromTheme("mail-message-new"), tr("Send text..."), this, SLOT(sendLinkByMail()))->setData(selectedText);
        m_menu->addSeparator();

        QString langCode = mApp->getActiveLanguage().left(2);
        QUrl googleTranslateUrl = QUrl(QString("http://translate.google.com/#auto|%1|%2").arg(langCode, selectedText));
        m_menu->addAction(QIcon(":icons/menu/translate.png"), tr("Google Translate"), this, SLOT(openUrlInNewTab()))->setData(googleTranslateUrl);
        m_menu->addAction(tr("Dictionary"), this, SLOT(openUrlInNewTab()))->setData("http://" + (langCode != "" ? langCode + "." : langCode) + "wiktionary.org/wiki/Special:Search?search=" + selectedText);
        m_menu->addSeparator();

        QString selectedString = selectedText.trimmed();
        if (!selectedString.contains(".")) {
            // Try to add .com
            selectedString.append(".com");
        }
        QUrl guessedUrl = QUrl::fromUserInput(selectedString);

        if (isUrlValid(guessedUrl)) {
            m_menu->addAction(QIcon(":/icons/menu/popup.png"), tr("Go to &web address"), this, SLOT(openUrlInNewTab()))->setData(guessedUrl);
        }

        selectedText.truncate(20);

        SearchEngine engine = mApp->searchEnginesManager()->activeEngine();
        m_menu->addAction(engine.icon, tr("Search \"%1 ..\" with %2").arg(selectedText, engine.name), this, SLOT(searchSelectedText()));
    }

#if (QTWEBKIT_VERSION >= QTWEBKIT_VERSION_CHECK(2, 2, 0))
//    still bugged? in 4.8 RC (it shows selection of webkit's internal source, not html from page)
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

    WebView::contextMenuEvent(event);
}

void TabbedWebView::stop()
{
    m_page->triggerAction(QWebPage::Stop);
    slotLoadFinished();
}

void TabbedWebView::openUrlInNewTab()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        m_tabWidget->addView(action->data().toUrl(), Qz::NT_NotSelectedTab);
    }
}

void TabbedWebView::searchSelectedText()
{
    SearchEngine engine = mApp->searchEnginesManager()->activeEngine();
    m_tabWidget->addView(engine.url.replace("%s", selectedText()), Qz::NT_NotSelectedTab);
}

void TabbedWebView::bookmarkLink()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        if (action->data().isNull()) {
            p_QupZilla->bookmarkPage();
        }
        else {
            p_QupZilla->addBookmark(action->data().toUrl(), title(), icon());
        }
    }
}

void TabbedWebView::showSourceOfSelection()
{
#if (QTWEBKIT_VERSION >= QTWEBKIT_VERSION_CHECK(2, 2, 0))
    showSource(m_page->mainFrame(), selectedHtml());
#endif
}

void TabbedWebView::showInspector()
{
    p_QupZilla->showWebInspector();
}

void TabbedWebView::getFocus(const QUrl &urla)
{
    if (urla == url()) {
        m_tabWidget->setCurrentWidget(m_webTab);
    }
}

// ClickedFrame slots

void TabbedWebView::loadClickedFrame()
{
    QUrl frameUrl = m_clickedFrame->url();
    if (frameUrl.isEmpty()) {
        frameUrl = m_clickedFrame->requestedUrl();
    }

    load(frameUrl);
}

void TabbedWebView::loadClickedFrameInNewTab()
{
    QUrl frameUrl = m_clickedFrame->url();
    if (frameUrl.isEmpty()) {
        frameUrl = m_clickedFrame->requestedUrl();
    }

    m_tabWidget->addView(frameUrl);
}

void TabbedWebView::reloadClickedFrame()
{
    QUrl frameUrl = m_clickedFrame->url();
    if (frameUrl.isEmpty()) {
        frameUrl = m_clickedFrame->requestedUrl();
    }

    m_clickedFrame->load(frameUrl);
}

void TabbedWebView::printClickedFrame()
{
    printPage(m_clickedFrame);
}

void TabbedWebView::clickedFrameZoomIn()
{
    qreal zFactor = m_clickedFrame->zoomFactor() + 0.1;
    if (zFactor > 2.5) {
        zFactor = 2.5;
    }

    m_clickedFrame->setZoomFactor(zFactor);
}

void TabbedWebView::clickedFrameZoomOut()
{
    qreal zFactor = m_clickedFrame->zoomFactor() - 0.1;
    if (zFactor < 0.5) {
        zFactor = 0.5;
    }

    m_clickedFrame->setZoomFactor(zFactor);
}

void TabbedWebView::clickedFrameZoomReset()
{
    m_clickedFrame->setZoomFactor(zoomFactor());
}

void TabbedWebView::showClickedFrameSource()
{
    showSource(m_clickedFrame);
}

void TabbedWebView::mousePressEvent(QMouseEvent* event)
{
    switch (event->button()) {
    case Qt::MiddleButton:
        if (isUrlValid(QUrl(m_hoveredLink))) {
            m_tabWidget->addView(QUrl::fromEncoded(m_hoveredLink.toUtf8()), Qz::NT_NotSelectedTab);
            event->accept();
            return;
        }
#ifdef Q_WS_WIN
        else {
            WebView::mouseDoubleClickEvent(event);
            return;
        }
#endif
        break;

    case Qt::LeftButton:
        if (event->modifiers() == Qt::ControlModifier && isUrlValid(QUrl(m_hoveredLink))) {
            m_tabWidget->addView(QUrl::fromEncoded(m_hoveredLink.toUtf8()), Qz::NT_NotSelectedTab);
            event->accept();
            return;
        }
    default:
        break;
    }

    WebView::mousePressEvent(event);
}

void TabbedWebView::mouseMoveEvent(QMouseEvent* event)
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
    WebView::mouseMoveEvent(event);
}

void TabbedWebView::disconnectObjects()
{
    disconnect(this);
    disconnect(p_QupZilla->statusBar());
}

TabbedWebView::~TabbedWebView()
{
}
