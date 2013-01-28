/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2013  David Rosca <nowrep@gmail.com>
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
#include "webpage.h"
#include "mainapplication.h"
#include "qztools.h"
#include "iconprovider.h"
#include "history.h"
#include "autofill.h"
#include "pluginproxy.h"
#include "downloadmanager.h"
#include "sourceviewer.h"
#include "siteinfo.h"
#include "searchenginesmanager.h"
#include "browsinglibrary.h"
#include "bookmarksmanager.h"
#include "settings.h"
#include "qzsettings.h"
#include "enhancedmenu.h"

#include <QDir>
#include <QTimer>
#include <QDesktopServices>
#include <QNetworkRequest>
#include <QWebHistory>
#include <QWebFrame>
#include <QClipboard>
#include <QTouchEvent>
#include <QPrintPreviewDialog>

WebView::WebView(QWidget* parent)
    : QWebView(parent)
    , m_currentZoom(100)
    , m_isLoading(false)
    , m_progress(0)
    , m_clickedFrame(0)
    , m_page(0)
    , m_actionReload(0)
    , m_actionStop(0)
    , m_actionsInitialized(false)
    , m_disableTouchMocking(false)
    , m_isReloading(false)
{
    connect(this, SIGNAL(loadStarted()), this, SLOT(slotLoadStarted()));
    connect(this, SIGNAL(loadProgress(int)), this, SLOT(slotLoadProgress(int)));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(slotLoadFinished()));
    connect(this, SIGNAL(iconChanged()), this, SLOT(slotIconChanged()));
    connect(this, SIGNAL(urlChanged(QUrl)), this, SLOT(slotUrlChanged(QUrl)));

    // Zoom levels same as in firefox
    m_zoomLevels << 30 << 50 << 67 << 80 << 90 << 100 << 110 << 120 << 133 << 150 << 170 << 200 << 240 << 300;

#if QTWEBKIT_TO_2_3
    installEventFilter(this);
#endif
}

QIcon WebView::icon() const
{
    if (url().scheme() == QLatin1String("qupzilla")) {
        return QIcon(":icons/qupzilla.png");
    }

    if (url().scheme() == QLatin1String("file")) {
        return qIconProvider->standardIcon(QStyle::SP_DriveHDIcon);
    }

    if (!QWebView::icon().isNull()) {
        return QWebView::icon();
    }

    if (!m_siteIcon.isNull() && m_siteIconUrl.host() == url().host()) {
        return m_siteIcon;
    }

    return _iconForUrl(url());
}

QString WebView::title() const
{
    QString title = QWebView::title();

    if (title.isEmpty()) {
        title = url().toString(QUrl::RemoveFragment);
    }

    if (title.isEmpty() || title == QLatin1String("about:blank")) {
        return tr("No Named Page");
    }

    return title;
}

QUrl WebView::url() const
{
    QUrl returnUrl = page()->url();

    if (returnUrl.isEmpty()) {
        returnUrl = m_aboutToLoadUrl;
    }

    if (returnUrl.toString() == QLatin1String("about:blank")) {
        returnUrl = QUrl();
    }

    return returnUrl;
}

WebPage* WebView::page() const
{
    return m_page;
}

void WebView::setPage(QWebPage* page)
{
    if (m_page == page) {
        return;
    }

    QWebView::setPage(page);
    m_page = qobject_cast<WebPage*>(page);

    setZoom(qzSettings->defaultZoom);
    connect(m_page, SIGNAL(saveFrameStateRequested(QWebFrame*, QWebHistoryItem*)), this, SLOT(frameStateChanged()));
    connect(m_page, SIGNAL(privacyChanged(bool)), this, SIGNAL(privacyChanged(bool)));

    mApp->plugins()->emitWebPageCreated(m_page);

    /* Set white background by default.
       Fixes issue with dark themes.

       See #602
     */
    QPalette pal = palette();
    pal.setBrush(QPalette::Base, Qt::white);
    page->setPalette(pal);

}

void WebView::load(const QUrl &url)
{
    WebView::load(QNetworkRequest(url));
}

void WebView::load(const QNetworkRequest &request, QNetworkAccessManager::Operation operation, const QByteArray &body)
{
    const QUrl &reqUrl = request.url();

    if (reqUrl.scheme() == QLatin1String("javascript")) {
        // Getting scriptSource from PercentEncoding to properly load bookmarklets
        QString scriptSource;
        if (reqUrl.path().contains(' '))
        {
            scriptSource = reqUrl.toString().mid(11).toUtf8();
        }
        else
        {
            scriptSource = QUrl::fromPercentEncoding(reqUrl.toString().mid(11).toUtf8());
        }
        page()->mainFrame()->evaluateJavaScript(scriptSource);
        return;
    }

    if (reqUrl.isEmpty() || isUrlValid(reqUrl)) {
        QWebView::load(request, operation, body);
        emit urlChanged(reqUrl);
        m_aboutToLoadUrl = reqUrl;
        return;
    }

    const QUrl &searchUrl = mApp->searchEnginesManager()->searchUrl(reqUrl.toString());
    QWebView::load(searchUrl);

    emit urlChanged(searchUrl);
    m_aboutToLoadUrl = searchUrl;
}

bool WebView::loadingError() const
{
    return page()->loadingError();
}

bool WebView::isLoading() const
{
    return m_isLoading;
}

int WebView::loadingProgress() const
{
    return m_progress;
}

void WebView::fakeLoadingProgress(int progress)
{
    emit loadStarted();
    emit loadProgress(progress);
}

bool WebView::isUrlValid(const QUrl &url)
{
    const QString &urlScheme = url.scheme();
    if (urlScheme == QLatin1String("data") || urlScheme == QLatin1String("qrc") ||
            urlScheme == QLatin1String("mailto")) {
        return true;
    }

    if (urlScheme == QLatin1String("qupzilla") || urlScheme == QLatin1String("file")) {
        return !url.path().isEmpty();
    }

    if (url.isValid() && !url.host().isEmpty() && !urlScheme.isEmpty()) {
        return true;
    }

    return false;
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

void WebView::addNotification(QWidget* notif)
{
    emit showNotification(notif);
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

void WebView::reload()
{
    m_isReloading = true;
    if (QWebView::url().isEmpty() && !m_aboutToLoadUrl.isEmpty()) {
        load(m_aboutToLoadUrl);
        return;
    }

    QWebView::reload();
}

void WebView::back()
{
    QWebHistory* history = page()->history();

    if (history->canGoBack()) {
        history->back();

        emit urlChanged(url());
        emit iconChanged();
    }
}

void WebView::forward()
{
    QWebHistory* history = page()->history();

    if (history->canGoForward()) {
        history->forward();

        emit urlChanged(url());
        emit iconChanged();
    }
}

void WebView::selectAll()
{
    triggerPageAction(QWebPage::SelectAll);
}

void WebView::slotLoadStarted()
{
    m_isLoading = true;
    m_progress = 0;

    if (m_actionsInitialized) {
        m_actionStop->setEnabled(true);
        m_actionReload->setEnabled(false);
    }
}

void WebView::slotLoadProgress(int progress)
{
    m_progress = progress;
}

void WebView::slotLoadFinished()
{
    m_isLoading = false;
    m_progress = 100;

    if (m_actionsInitialized) {
        m_actionStop->setEnabled(false);
        m_actionReload->setEnabled(true);
    }

    if (!m_isReloading) {
        mApp->history()->addHistoryEntry(this);
    }

    mApp->autoFill()->completePage(page());

    m_isReloading = false;
    m_lastUrl = url();
}

void WebView::frameStateChanged()
{
    // QWebFrame::baseUrl() is not updated yet, so we are invoking 0 second timer
    QTimer::singleShot(0, this, SLOT(emitChangedUrl()));
}

void WebView::emitChangedUrl()
{
    emit urlChanged(url());
}

void WebView::slotIconChanged()
{
    if (!loadingError()) {
        m_siteIcon = icon();
        m_siteIconUrl = url();

        qIconProvider->saveIcon(this);
    }
}

void WebView::slotUrlChanged(const QUrl &url)
{
    static QStringList exceptions;
    if (exceptions.isEmpty()) {
        exceptions << "google." << "twitter.";
    }

    // Disable touch mocking on pages known not to work properly
    const QString &host = url.host();
    m_disableTouchMocking = false;

    foreach(const QString & site, exceptions) {
        if (host.contains(site)) {
            m_disableTouchMocking = true;
        }
    }
}

void WebView::openUrlInNewWindow()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        mApp->makeNewWindow(Qz::BW_NewWindow, action->data().toUrl());
    }
}

void WebView::sendLinkByMail()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        const QUrl &mailUrl = QUrl::fromEncoded("mailto:?body=" + QUrl::toPercentEncoding(action->data().toUrl().toEncoded()));
        QDesktopServices::openUrl(mailUrl);
    }
}

void WebView::sendPageByMail()
{
    const QUrl &mailUrl = QUrl::fromEncoded("mailto:?body=" + QUrl::toPercentEncoding(url().toEncoded()) + "&subject=" + QUrl::toPercentEncoding(title()));
    QDesktopServices::openUrl(mailUrl);
}

void WebView::copyLinkToClipboard()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        QApplication::clipboard()->setText(action->data().toString());
    }
}

void WebView::savePageAs()
{
    QNetworkRequest request(url());
    QString suggestedFileName = QzTools::getFileNameFromUrl(url());
    if (!suggestedFileName.contains(QLatin1Char('.'))) {
        suggestedFileName.append(QLatin1String(".html"));
    }

    DownloadManager::DownloadInfo info;
    info.page = page();
    info.suggestedFileName = suggestedFileName;
    info.askWhatToDo = false;
    info.forceChoosingPath = true;

    DownloadManager* dManager = mApp->downManager();
    dManager->download(request, info);
}

void WebView::downloadUrlToDisk()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        QNetworkRequest request(action->data().toUrl());

        DownloadManager::DownloadInfo info;
        info.page = page();
        info.suggestedFileName = QString();
        info.askWhatToDo = false;
        info.forceChoosingPath = true;

        DownloadManager* dManager = mApp->downManager();
        dManager->download(request, info);
    }
}

void WebView::copyImageToClipboard()
{
    triggerPageAction(QWebPage::CopyImageToClipboard);
}

void WebView::openActionUrl()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        load(action->data().toUrl());
    }
}

void WebView::showSource(QWebFrame* frame, const QString &selectedHtml)
{
    if (!frame) {
        frame = page()->mainFrame();
    }

    SourceViewer* source = new SourceViewer(frame, selectedHtml);
    QzTools::centerWidgetToParent(source, this);
    source->show();
}

void WebView::showSiteInfo()
{
    SiteInfo* s = new SiteInfo(this, this);
    s->show();
}

void WebView::searchSelectedText()
{
    SearchEngine engine = mApp->searchEnginesManager()->activeEngine();
    if (QAction* act = qobject_cast<QAction*>(sender())) {
        if (act->data().isValid()) {
            engine = act->data().value<SearchEngine>();
        }
    }

    const QUrl &urlToLoad = mApp->searchEnginesManager()->searchUrl(engine, selectedText());
    openUrlInNewTab(urlToLoad, Qz::NT_SelectedTab);
}

void WebView::searchSelectedTextInBackgroundTab()
{
    SearchEngine engine = mApp->searchEnginesManager()->activeEngine();
    if (QAction* act = qobject_cast<QAction*>(sender())) {
        if (act->data().isValid()) {
            engine = act->data().value<SearchEngine>();
        }
    }

    const QUrl &urlToLoad = mApp->searchEnginesManager()->searchUrl(engine, selectedText());
    openUrlInNewTab(urlToLoad, Qz::NT_NotSelectedTab);
}

void WebView::bookmarkLink()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        if (action->data().isNull()) {
            mApp->browsingLibrary()->bookmarksManager()->addBookmark(this);
        }
        else {
            mApp->browsingLibrary()->bookmarksManager()->insertBookmark(action->data().toUrl(), title(), icon());
        }
    }
}

void WebView::showSourceOfSelection()
{
#if QTWEBKIT_FROM_2_2
    showSource(page()->mainFrame(), selectedHtml());
#endif
}

void WebView::openUrlInSelectedTab()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        openUrlInNewTab(action->data().toUrl(), Qz::NT_SelectedTab);
    }
}

void WebView::openUrlInBackgroundTab()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        openUrlInNewTab(action->data().toUrl(), Qz::NT_NotSelectedTab);
    }
}

void WebView::userDefinedOpenUrlInNewTab(const QUrl &url, bool invert)
{
    Qz::NewTabPositionFlag position = qzSettings->newTabPosition;
    if (invert) {
        position = (position == Qz::NT_SelectedTab) ? Qz::NT_NotSelectedTab : Qz::NT_SelectedTab;
    }

    QUrl actionUrl;

    if (!url.isEmpty()) {
        actionUrl = url;
    }
    else if (QAction* action = qobject_cast<QAction*>(sender())) {
        actionUrl = action->data().toUrl();
    }

    openUrlInNewTab(actionUrl, position);
}

void WebView::userDefinedOpenUrlInBgTab(const QUrl &url)
{
    QUrl actionUrl;

    if (!url.isEmpty()) {
        actionUrl = url;
    }
    else if (QAction* action = qobject_cast<QAction*>(sender())) {
        actionUrl = action->data().toUrl();
    }

    userDefinedOpenUrlInNewTab(actionUrl, true);
}

void WebView::loadClickedFrame()
{
    QUrl frameUrl = m_clickedFrame->baseUrl();
    if (frameUrl.isEmpty()) {
        frameUrl = m_clickedFrame->requestedUrl();
    }

    load(frameUrl);
}

void WebView::loadClickedFrameInNewTab(bool invert)
{
    QUrl frameUrl = m_clickedFrame->baseUrl();
    if (frameUrl.isEmpty()) {
        frameUrl = m_clickedFrame->requestedUrl();
    }

    userDefinedOpenUrlInNewTab(frameUrl, invert);
}

void WebView::loadClickedFrameInBgTab()
{
    loadClickedFrameInNewTab(true);
}

void WebView::reloadClickedFrame()
{
    QUrl frameUrl = m_clickedFrame->baseUrl();
    if (frameUrl.isEmpty()) {
        frameUrl = m_clickedFrame->requestedUrl();
    }

    m_clickedFrame->load(frameUrl);
}

void WebView::printClickedFrame()
{
    printPage(m_clickedFrame);
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
    showSource(m_clickedFrame);
}

void WebView::printPage(QWebFrame* frame)
{
    QPrintPreviewDialog* dialog = new QPrintPreviewDialog(this);
    dialog->resize(800, 750);

    if (!frame) {
        connect(dialog, SIGNAL(paintRequested(QPrinter*)), this, SLOT(print(QPrinter*)));
    }
    else {
        connect(dialog, SIGNAL(paintRequested(QPrinter*)), frame, SLOT(print(QPrinter*)));
    }

    dialog->exec();
    dialog->deleteLater();
}

QUrl WebView::lastUrl()
{
    return m_lastUrl;
}

bool WebView::isMediaElement(const QWebElement &element)
{
    return (element.tagName().toLower() == QLatin1String("video")
            || element.tagName().toLower() == QLatin1String("audio"));
}

void WebView::checkForForm(QMenu* menu, const QWebElement &element)
{
    QWebElement parentElement = element.parent();

    while (!parentElement.isNull()) {
        if (parentElement.tagName().toLower() == QLatin1String("form")) {
            break;
        }

        parentElement = parentElement.parent();
    }

    if (parentElement.isNull()) {
        return;
    }

    const QString &url = parentElement.attribute("action");
    const QString &method = parentElement.hasAttribute("method") ? parentElement.attribute("method").toUpper() : "GET";

    if (!url.isEmpty() && method == QLatin1String("GET")) {
        menu->addAction(QIcon(":icons/menu/search-icon.png"), tr("Create Search Engine"), this, SLOT(createSearchEngine()));

        m_clickedElement = element;
    }
}

void WebView::createSearchEngine()
{
    mApp->searchEnginesManager()->addEngineFromForm(m_clickedElement, this);
}

void WebView::createContextMenu(QMenu* menu, const QWebHitTestResult &hitTest, const QPoint &pos)
{
    if (!m_actionsInitialized) {
        m_actionsInitialized = true;

        pageAction(QWebPage::Cut)->setIcon(QIcon::fromTheme("edit-cut"));
        pageAction(QWebPage::Copy)->setIcon(QIcon::fromTheme("edit-copy"));
        pageAction(QWebPage::Paste)->setIcon(QIcon::fromTheme("edit-paste"));
        pageAction(QWebPage::SelectAll)->setIcon(QIcon::fromTheme("edit-select-all"));

        m_actionReload = new QAction(qIconProvider->standardIcon(QStyle::SP_BrowserReload), tr("&Reload"), this);
        m_actionStop = new QAction(qIconProvider->standardIcon(QStyle::SP_BrowserStop), tr("S&top"), this);

        connect(m_actionReload, SIGNAL(triggered()), this, SLOT(reload()));
        connect(m_actionStop, SIGNAL(triggered()), this, SLOT(stop()));

        m_actionReload->setEnabled(!isLoading());
        m_actionStop->setEnabled(isLoading());
    }

    if (!hitTest.linkUrl().isEmpty() && hitTest.linkUrl().scheme() != QLatin1String("javascript")) {
        createLinkContextMenu(menu, hitTest);
    }

    if (!hitTest.imageUrl().isEmpty()) {
        createImageContextMenu(menu, hitTest);
    }

    if (isMediaElement(hitTest.element())) {
        createMediaContextMenu(menu, hitTest);
    }

    if (hitTest.isContentEditable()) {
        if (menu->isEmpty()) {
            QMenu* pageMenu = page()->createStandardContextMenu();

            int i = 0;
            foreach(QAction * act, pageMenu->actions()) {
                if (act->isSeparator()) {
                    menu->addSeparator();
                    continue;
                }

                // Hiding double Direction + Fonts menu (bug in QtWebKit 2.2)
                if (i <= 1 && act->menu()) {
                    if (act->menu()->actions().contains(pageAction(QWebPage::SetTextDirectionDefault)) ||
                            act->menu()->actions().contains(pageAction(QWebPage::ToggleBold))) {
                        act->setVisible(false);
                    }
                }

                menu->addAction(act);

                ++i;
            }

            if (menu->actions().last() == pageAction(QWebPage::InspectElement)) {
                // We have own Inspect Element action
                menu->actions().last()->setVisible(false);
            }

            delete pageMenu;
        }

        if (hitTest.element().tagName().toLower() == QLatin1String("input")) {
            checkForForm(menu, hitTest.element());
        }
    }

    if (!selectedText().isEmpty()) {
        createSelectedTextContextMenu(menu, hitTest);
    }

    if (menu->isEmpty()) {
        createPageContextMenu(menu, pos);
    }

    menu->addSeparator();
    mApp->plugins()->populateWebViewMenu(menu, this, hitTest);

#if QTWEBKIT_FROM_2_2
    //    still bugged? in 4.8 RC (it shows selection of webkit's internal source, not html from page)
    //    it may or may not be bug, but this implementation is useless for us
    //
    //    if (!selectedHtml().isEmpty())
    //        menu->addAction(tr("Show source of selection"), this, SLOT(showSourceOfSelection()));
#endif
}

void WebView::createPageContextMenu(QMenu* menu, const QPoint &pos)
{
    QWebFrame* frameAtPos = page()->frameAt(pos);

    QAction* action = menu->addAction(tr("&Back"), this, SLOT(back()));
    action->setIcon(qIconProvider->standardIcon(QStyle::SP_ArrowBack));
    action->setEnabled(history()->canGoBack());

    action = menu->addAction(tr("&Forward"), this, SLOT(forward()));
    action->setIcon(qIconProvider->standardIcon(QStyle::SP_ArrowForward));
    action->setEnabled(history()->canGoForward());

    menu->addAction(m_actionReload);
    menu->addAction(m_actionStop);
    menu->addSeparator();

    if (frameAtPos && page()->mainFrame() != frameAtPos) {
        m_clickedFrame = frameAtPos;
        Menu* frameMenu = new Menu(tr("This frame"));
        frameMenu->addAction(tr("Show &only this frame"), this, SLOT(loadClickedFrame()));
        Action* act = new Action(QIcon(":/icons/menu/new-tab.png"), tr("Show this frame in new &tab"));
        connect(act, SIGNAL(triggered()), this, SLOT(loadClickedFrameInNewTab()));
        connect(act, SIGNAL(middleClicked()), this, SLOT(loadClickedFrameInBgTab()));
        frameMenu->addAction(act);
        frameMenu->addSeparator();
        frameMenu->addAction(qIconProvider->standardIcon(QStyle::SP_BrowserReload), tr("&Reload"), this, SLOT(reloadClickedFrame()));
        frameMenu->addAction(QIcon::fromTheme("document-print"), tr("Print frame"), this, SLOT(printClickedFrame()));
        frameMenu->addSeparator();
        frameMenu->addAction(QIcon::fromTheme("zoom-in"), tr("Zoom &in"), this, SLOT(clickedFrameZoomIn()));
        frameMenu->addAction(QIcon::fromTheme("zoom-out"), tr("&Zoom out"), this, SLOT(clickedFrameZoomOut()));
        frameMenu->addAction(QIcon::fromTheme("zoom-original"), tr("Reset"), this, SLOT(clickedFrameZoomReset()));
        frameMenu->addSeparator();
        frameMenu->addAction(QIcon::fromTheme("text-html"), tr("Show so&urce of frame"), this, SLOT(showClickedFrameSource()));

        menu->addMenu(frameMenu);
    }

    menu->addSeparator();
    menu->addAction(qIconProvider->fromTheme("user-bookmarks"), tr("Book&mark page"), this, SLOT(bookmarkLink()));
    menu->addAction(QIcon::fromTheme("document-save"), tr("&Save page as..."), this, SLOT(savePageAs()));
    menu->addAction(QIcon::fromTheme("edit-copy"), tr("&Copy page link"), this, SLOT(copyLinkToClipboard()))->setData(url());
    menu->addAction(QIcon::fromTheme("mail-message-new"), tr("Send page link..."), this, SLOT(sendPageByMail()));
    menu->addAction(QIcon::fromTheme("document-print"), tr("&Print page"), this, SLOT(printPage()));
    menu->addSeparator();
    menu->addAction(QIcon::fromTheme("edit-select-all"), tr("Select &all"), this, SLOT(selectAll()));
    menu->addSeparator();
    if (url().scheme() == QLatin1String("http") || url().scheme() == QLatin1String("https")) {
//        bool result = validateConfirm(tr("Do you want to upload this page to an online source code validator?"));
//        if (result)
        menu->addAction(tr("Validate page"), this, SLOT(openUrlInSelectedTab()))->setData(QUrl("http://validator.w3.org/check?uri=" + url().toString()));
    }

    menu->addAction(QIcon::fromTheme("text-html"), tr("Show so&urce code"), this, SLOT(showSource()));
    menu->addAction(QIcon::fromTheme("dialog-information"), tr("Show info ab&out site"), this, SLOT(showSiteInfo()));
}

void WebView::createLinkContextMenu(QMenu* menu, const QWebHitTestResult &hitTest)
{
    // Workaround for QtWebKit <= 2.0 when selecting link text on right click
    if (page()->selectedText() == hitTest.linkText()) {
        findText(QString());
    }

    menu->addSeparator();
    Action* act = new Action(QIcon(":/icons/menu/new-tab.png"), tr("Open link in new &tab"));
    act->setData(hitTest.linkUrl());
    connect(act, SIGNAL(triggered()), this, SLOT(userDefinedOpenUrlInNewTab()));
    connect(act, SIGNAL(middleClicked()), this, SLOT(userDefinedOpenUrlInBgTab()));
    menu->addAction(act);
    menu->addAction(QIcon::fromTheme("window-new"), tr("Open link in new &window"), this, SLOT(openUrlInNewWindow()))->setData(hitTest.linkUrl());
    menu->addSeparator();
    menu->addAction(qIconProvider->fromTheme("user-bookmarks"), tr("B&ookmark link"), this, SLOT(bookmarkLink()))->setData(hitTest.linkUrl());
    menu->addAction(QIcon::fromTheme("document-save"), tr("&Save link as..."), this, SLOT(downloadUrlToDisk()))->setData(hitTest.linkUrl());
    menu->addAction(QIcon::fromTheme("mail-message-new"), tr("Send link..."), this, SLOT(sendLinkByMail()))->setData(hitTest.linkUrl());
    menu->addAction(QIcon::fromTheme("edit-copy"), tr("&Copy link address"), this, SLOT(copyLinkToClipboard()))->setData(hitTest.linkUrl());
    menu->addSeparator();

    if (!selectedText().isEmpty()) {
        pageAction(QWebPage::Copy)->setIcon(QIcon::fromTheme("edit-copy"));
        menu->addAction(pageAction(QWebPage::Copy));
    }
}

void WebView::createImageContextMenu(QMenu* menu, const QWebHitTestResult &hitTest)
{
    menu->addSeparator();
    Action* act = new Action(tr("Show i&mage"));
    act->setData(hitTest.imageUrl());
    connect(act, SIGNAL(triggered()), this, SLOT(openActionUrl()));
    connect(act, SIGNAL(middleClicked()), this, SLOT(userDefinedOpenUrlInNewTab()));
    menu->addAction(act);
    menu->addAction(tr("Copy im&age"), this, SLOT(copyImageToClipboard()))->setData(hitTest.imageUrl());
    menu->addAction(QIcon::fromTheme("edit-copy"), tr("Copy image ad&dress"), this, SLOT(copyLinkToClipboard()))->setData(hitTest.imageUrl());
    menu->addSeparator();
    menu->addAction(QIcon::fromTheme("document-save"), tr("&Save image as..."), this, SLOT(downloadUrlToDisk()))->setData(hitTest.imageUrl());
    menu->addAction(QIcon::fromTheme("mail-message-new"), tr("Send image..."), this, SLOT(sendLinkByMail()))->setData(hitTest.imageUrl());
    menu->addSeparator();

    if (!selectedText().isEmpty()) {
        pageAction(QWebPage::Copy)->setIcon(QIcon::fromTheme("edit-copy"));
        menu->addAction(pageAction(QWebPage::Copy));
    }
}

void WebView::createSelectedTextContextMenu(QMenu* menu, const QWebHitTestResult &hitTest)
{
    Q_UNUSED(hitTest)

    QString selectedText = page()->selectedText();

    menu->addSeparator();
    if (!menu->actions().contains(pageAction(QWebPage::Copy))) {
        menu->addAction(pageAction(QWebPage::Copy));
    }
    menu->addAction(QIcon::fromTheme("mail-message-new"), tr("Send text..."), this, SLOT(sendLinkByMail()))->setData(selectedText);
    menu->addSeparator();

    QString langCode = mApp->currentLanguage().left(2);
    QUrl googleTranslateUrl = QUrl(QString("http://translate.google.com/#auto|%1|%2").arg(langCode, selectedText));
    Action* gtwact = new Action(QIcon(":icons/sites/translate.png"), tr("Google Translate"));
    gtwact->setData(googleTranslateUrl);
    connect(gtwact, SIGNAL(triggered()), this, SLOT(openUrlInSelectedTab()));
    connect(gtwact, SIGNAL(middleClicked()), this, SLOT(openUrlInBackgroundTab()));
    menu->addAction(gtwact);
    Action* dictact = new Action(QIcon::fromTheme("accessories-dictionary"), tr("Dictionary"));
    dictact->setData(QUrl("http://" + (!langCode.isEmpty() ? langCode + "." : langCode) + "wiktionary.org/wiki/Special:Search?search=" + selectedText));
    connect(dictact, SIGNAL(triggered()), this, SLOT(openUrlInSelectedTab()));
    connect(dictact, SIGNAL(middleClicked()), this, SLOT(openUrlInBackgroundTab()));
    menu->addAction(dictact);

    // #379: Remove newlines
    QString selectedString = selectedText.trimmed().remove(QLatin1Char('\n'));
    if (!selectedString.contains(QLatin1Char('.'))) {
        // Try to add .com
        selectedString.append(QLatin1String(".com"));
    }
    QUrl guessedUrl = QUrl::fromUserInput(selectedString);

    if (isUrlValid(guessedUrl)) {
        Action* act = new Action(QIcon::fromTheme("document-open-remote"), tr("Go to &web address"));
        act->setData(guessedUrl);

        connect(act, SIGNAL(triggered()), this, SLOT(openActionUrl()));
        connect(act, SIGNAL(middleClicked()), this, SLOT(userDefinedOpenUrlInNewTab()));
        menu->addAction(act);
    }

    menu->addSeparator();
    selectedText.truncate(20);
    // KDE is displaying new lines in menu actions ... weird -,-
    selectedText.replace(QLatin1Char('\n'), QLatin1Char(' ')).remove(QLatin1Char('\t'));

    SearchEngine engine = mApp->searchEnginesManager()->activeEngine();
    Action* act = new Action(engine.icon, tr("Search \"%1 ..\" with %2").arg(selectedText, engine.name));
    connect(act, SIGNAL(triggered()), this, SLOT(searchSelectedText()));
    connect(act, SIGNAL(middleClicked()), this, SLOT(searchSelectedTextInBackgroundTab()));
    menu->addAction(act);

    // Search with ...
    Menu* swMenu = new Menu(tr("Search with..."), menu);
    SearchEnginesManager* searchManager = mApp->searchEnginesManager();
    foreach(const SearchEngine & en, searchManager->allEngines()) {
        Action* act = new Action(en.icon, en.name);
        act->setData(qVariantFromValue(en));

        connect(act, SIGNAL(triggered()), this, SLOT(searchSelectedText()));
        connect(act, SIGNAL(middleClicked()), this, SLOT(searchSelectedTextInBackgroundTab()));
        swMenu->addAction(act);
    }

    menu->addMenu(swMenu);
}

void WebView::createMediaContextMenu(QMenu* menu, const QWebHitTestResult &hitTest)
{
    m_clickedElement = hitTest.element();

    if (m_clickedElement.isNull()) {
        return;
    }

    bool paused = m_clickedElement.evaluateJavaScript("this.paused").toBool();
    bool muted = m_clickedElement.evaluateJavaScript("this.muted").toBool();
    QUrl videoUrl = m_clickedElement.evaluateJavaScript("this.currentSrc").toUrl();

    menu->addSeparator();
    menu->addAction(paused ? tr("&Play") : tr("&Pause"), this, SLOT(pauseMedia()))->setIcon(QIcon::fromTheme(paused ? "media-playback-start" : "media-playback-pause"));
    menu->addAction(muted ? tr("Un&mute") : tr("&Mute"), this, SLOT(muteMedia()))->setIcon(QIcon::fromTheme(muted ? "audio-volume-muted" : "audio-volume-high"));
    menu->addSeparator();
    menu->addAction(QIcon::fromTheme("edit-copy"), tr("&Copy Media Address"), this, SLOT(copyLinkToClipboard()))->setData(videoUrl);
    menu->addAction(QIcon::fromTheme("mail-message-new"), tr("&Send Media Address"), this, SLOT(sendLinkByMail()))->setData(videoUrl);
    menu->addAction(QIcon::fromTheme("document-save"), tr("Save Media To &Disk"), this, SLOT(downloadUrlToDisk()))->setData(videoUrl);
}

void WebView::pauseMedia()
{
    bool paused = m_clickedElement.evaluateJavaScript("this.paused").toBool();

    if (paused) {
        m_clickedElement.evaluateJavaScript("this.play()");
    }
    else {
        m_clickedElement.evaluateJavaScript("this.pause()");
    }
}

void WebView::muteMedia()
{
    bool muted = m_clickedElement.evaluateJavaScript("this.muted").toBool();

    if (muted) {
        m_clickedElement.evaluateJavaScript("this.muted = false");
    }
    else {
        m_clickedElement.evaluateJavaScript("this.muted = true");
    }
}

void WebView::wheelEvent(QWheelEvent* event)
{
    if (mApp->plugins()->processWheelEvent(Qz::ON_WebView, this, event)) {
        return;
    }

    if (event->modifiers() & Qt::ControlModifier) {
        event->delta() > 0 ? zoomIn() : zoomOut();
        event->accept();

        return;
    }

    QWebView::wheelEvent(event);
}

void WebView::mousePressEvent(QMouseEvent* event)
{
    if (mApp->plugins()->processMousePress(Qz::ON_WebView, this, event)) {
        return;
    }

    switch (event->button()) {
    case Qt::XButton1:
        back();
        event->accept();
        break;

    case Qt::XButton2:
        forward();
        event->accept();
        break;

    case Qt::MiddleButton: {
        QWebFrame* frame = page()->frameAt(event->pos());
        if (frame) {
            m_clickedUrl = frame->hitTestContent(event->pos()).linkUrl();
            if (!m_clickedUrl.isEmpty()) {
                return;
            }
        }

        break;
    }

    case Qt::LeftButton: {
        QWebFrame* frame = page()->frameAt(event->pos());
        if (frame) {
            const QUrl &link = frame->hitTestContent(event->pos()).linkUrl();
            if (event->modifiers() & Qt::ControlModifier && isUrlValid(link)) {
                userDefinedOpenUrlInNewTab(link, event->modifiers() & Qt::ShiftModifier);
                event->accept();
                return;
            }
        }
    }

    default:
        break;
    }

    QWebView::mousePressEvent(event);
}

void WebView::mouseReleaseEvent(QMouseEvent* event)
{
    if (mApp->plugins()->processMouseRelease(Qz::ON_WebView, this, event)) {
        return;
    }

    switch (event->button()) {
    case Qt::MiddleButton: {
        QWebFrame* frame = page()->frameAt(event->pos());
        if (frame) {
            const QUrl &link = frame->hitTestContent(event->pos()).linkUrl();
            if (m_clickedUrl == link && isUrlValid(link)) {
                userDefinedOpenUrlInNewTab(link, event->modifiers() & Qt::ShiftModifier);
                event->accept();
                return;
            }
        }

        break;
    }

    default:
        break;
    }

    QWebView::mouseReleaseEvent(event);
}

void WebView::mouseMoveEvent(QMouseEvent* event)
{
    if (mApp->plugins()->processMouseMove(Qz::ON_WebView, this, event)) {
        return;
    }

    QWebView::mouseMoveEvent(event);
}

void WebView::keyPressEvent(QKeyEvent* event)
{
    if (mApp->plugins()->processKeyPress(Qz::ON_WebView, this, event)) {
        return;
    }

    switch (event->key()) {
    case Qt::Key_C:
        if (event->modifiers() == Qt::ControlModifier) {
            triggerPageAction(QWebPage::Copy);
            event->accept();
            return;
        }
        break;

    case Qt::Key_A:
        if (event->modifiers() == Qt::ControlModifier) {
            selectAll();
            event->accept();
            return;
        }
        break;

    default:
        break;
    }

    QWebView::keyPressEvent(event);
}

void WebView::keyReleaseEvent(QKeyEvent* event)
{
    if (mApp->plugins()->processKeyRelease(Qz::ON_WebView, this, event)) {
        return;
    }

    QWebView::keyReleaseEvent(event);
}

void WebView::resizeEvent(QResizeEvent* event)
{
    QWebView::resizeEvent(event);
    emit viewportResized(page()->viewportSize());
}

void WebView::setZoom(int zoom)
{
    m_currentZoom = zoom;
    applyZoom();
}

///
// This function was taken and modified from QTestBrowser to fix bug #33 with flightradar24.com
// You can find original source and copyright here:
// http://gitorious.org/+qtwebkit-developers/webkit/qtwebkit/blobs/qtwebkit-2.2/Tools/QtTestBrowser/launcherwindow.cpp
///
bool WebView::eventFilter(QObject* obj, QEvent* event)
{
// This hack is no longer needed with QtWebKit 2.3 (bundled in Qt 5)
#if QTWEBKIT_TO_2_3
    if (obj != this || m_disableTouchMocking) {
        return false;
    }

    if (event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::MouseButtonRelease ||
            event->type() == QEvent::MouseButtonDblClick ||
            event->type() == QEvent::MouseMove) {

        QMouseEvent* ev = static_cast<QMouseEvent*>(event);

        if (ev->type() == QEvent::MouseMove && !(ev->buttons() & Qt::LeftButton)) {
            return false;
        }

        if (ev->type() == QEvent::MouseButtonPress && !(ev->buttons() & Qt::LeftButton)) {
            return false;
        }

        QEvent::Type type = QEvent::TouchUpdate;
        QTouchEvent::TouchPoint touchPoint;
        touchPoint.setId(0);
        touchPoint.setScreenPos(ev->globalPos());
        touchPoint.setPos(ev->pos());
        touchPoint.setPressure(1);

        switch (ev->type()) {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonDblClick:
            touchPoint.setState(Qt::TouchPointPressed);
            type = QEvent::TouchBegin;

            break;

        case QEvent::MouseButtonRelease:
            touchPoint.setState(Qt::TouchPointReleased);
            type = QEvent::TouchEnd;

            break;

        case QEvent::MouseMove:
            touchPoint.setState(Qt::TouchPointMoved);
            type = QEvent::TouchUpdate;

            break;

        default:
            break;
        }

        QList<QTouchEvent::TouchPoint> touchPoints;
        touchPoints << touchPoint;

        QTouchEvent touchEv(type);
        touchEv.setTouchPoints(touchPoints);
        QCoreApplication::sendEvent(page(), &touchEv);

        return false;
    }
#endif
    return QWebView::eventFilter(obj, event);
}

void WebView::disconnectObjects()
{
    disconnect(this);
}
