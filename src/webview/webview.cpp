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
#include "webpage.h"
#include "webhistorywrapper.h"
#include "mainapplication.h"
#include "globalfunctions.h"
#include "iconprovider.h"
#include "historymodel.h"
#include "autofillmodel.h"
#include "downloadmanager.h"
#include "sourceviewer.h"
#include "siteinfo.h"
#include "searchenginesmanager.h"
#include "browsinglibrary.h"
#include "bookmarksmanager.h"
#include "settings.h"
#include "webviewsettings.h"

WebView::WebView(QWidget* parent)
    : QWebView(parent)
    , m_currentZoom(100)
    , m_isLoading(false)
    , m_progress(0)
    , m_clickedFrame(0)
    , m_actionsHaveImages(false)
{
    connect(this, SIGNAL(loadStarted()), this, SLOT(slotLoadStarted()));
    connect(this, SIGNAL(loadProgress(int)), this, SLOT(slotLoadProgress(int)));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(slotLoadFinished()));

    connect(this, SIGNAL(iconChanged()), this, SLOT(slotIconChanged()));

    // Zoom levels same as in firefox
    m_zoomLevels << 30 << 50 << 67 << 80 << 90 << 100 << 110 << 120 << 133 << 150 << 170 << 200 << 240 << 300;

    qApp->installEventFilter(this);
}

QIcon WebView::icon() const
{
    if (url().scheme() == "qupzilla") {
        return QIcon(":icons/qupzilla.png");
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

    if (title.isEmpty() || title == "about:blank") {
        return tr("No Named Page");
    }

    return title;
}

QUrl WebView::url() const
{
    QUrl returnUrl = QWebView::url();

    if (returnUrl.isEmpty()) {
        returnUrl = m_aboutToLoadUrl;
    }

    return returnUrl;
}

void WebView::setPage(QWebPage *page)
{
    QWebView::setPage(page);

    setZoom(WebViewSettings::defaultZoom);
}

void WebView::load(const QUrl &url)
{
    if (url.scheme() == "javascript") {
        page()->mainFrame()->evaluateJavaScript(url.toString());
        return;
    }

    if (isUrlValid(url)) {
        QWebView::load(url);
        emit urlChanged(url);
        m_aboutToLoadUrl = url;
        return;
    }

    QUrl searchUrl = mApp->searchEnginesManager()->searchUrl(url.toString());
    QWebView::load(searchUrl);
    emit urlChanged(searchUrl);
    m_aboutToLoadUrl = searchUrl;
}

bool WebView::isLoading() const
{
    return m_isLoading;
}

int WebView::loadProgress() const
{
    return m_progress;
}

bool WebView::isUrlValid(const QUrl &url)
{
    if (url.scheme() == "data" || url.scheme() == "qrc" || url.scheme() == "mailto") {
        return true;
    }

    if (url.scheme() == "qupzilla" || url.scheme() == "file") {
        return !url.path().isEmpty();
    }

    if (url.isValid() && !url.host().isEmpty() && !url.scheme().isEmpty()) {
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
    if (QWebView::url().isEmpty() && !m_aboutToLoadUrl.isEmpty()) {
        load(m_aboutToLoadUrl);
        return;
    }

    QWebView::reload();
}

void WebView::back()
{
    QWebHistory* history = page()->history();

    if (WebHistoryWrapper::canGoBack(history)) {
        WebHistoryWrapper::goBack(history);

        emit urlChanged(url());
        emit iconChanged();
    }
}

void WebView::forward()
{
    QWebHistory* history = page()->history();

    if (WebHistoryWrapper::canGoForward(history)) {
        WebHistoryWrapper::goForward(history);

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
}

void WebView::slotLoadProgress(int progress)
{
    m_progress = progress;
}

void WebView::slotLoadFinished()
{
    m_isLoading = false;
    m_progress = 100;

    if (m_lastUrl != url()) {
        mApp->history()->addHistoryEntry(this);
    }

    mApp->autoFill()->completePage(qobject_cast<WebPage*>(page()));

    m_lastUrl = url();
}

void WebView::slotIconChanged()
{
    m_siteIcon = icon();
    m_siteIconUrl = url();
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
        QDesktopServices::openUrl(QUrl("mailto:?body=" + action->data().toString()));
    }
}

void WebView::copyLinkToClipboard()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        QApplication::clipboard()->setText(action->data().toString());
    }
}

void WebView::downloadLinkToDisk()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        DownloadManager* dManager = mApp->downManager();
        QNetworkRequest request(action->data().toUrl());
        dManager->download(request, qobject_cast<WebPage*>(page()), false);
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
    qz_centerWidgetToParent(source, this);
    source->show();
}

void WebView::showSiteInfo()
{
    SiteInfo* s = new SiteInfo(this, this);
    s->show();
}

void WebView::searchSelectedText()
{
    QUrl urlToLoad = mApp->searchEnginesManager()->searchUrl(selectedText());

    openUrlInNewTab(urlToLoad, Qz::NT_SelectedTab);
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
#if (QTWEBKIT_VERSION >= QTWEBKIT_VERSION_CHECK(2, 2, 0))
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
        openUrlInNewTab(action->data().toUrl(), WebViewSettings::newTabAfterActive ? Qz::NT_NotSelectedTab : Qz::NT_CleanTab);
    }
}

void WebView::loadClickedFrame()
{
    QUrl frameUrl = m_clickedFrame->url();
    if (frameUrl.isEmpty()) {
        frameUrl = m_clickedFrame->requestedUrl();
    }

    load(frameUrl);
}

void WebView::loadClickedFrameInNewTab()
{
    QUrl frameUrl = m_clickedFrame->url();
    if (frameUrl.isEmpty()) {
        frameUrl = m_clickedFrame->requestedUrl();
    }

    openUrlInNewTab(frameUrl, Qz::NT_SelectedTab);
}

void WebView::reloadClickedFrame()
{
    QUrl frameUrl = m_clickedFrame->url();
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

void WebView::copyText()
{
    if (!selectedText().isEmpty()) {
        QApplication::clipboard()->setText(selectedText());
    }
}

QUrl WebView::lastUrl()
{
    return m_lastUrl;
}

bool WebView::isMediaElement(const QWebElement &element)
{
    return (element.tagName().toLower() == "video" || element.tagName().toLower() == "audio");
}

void WebView::createContextMenu(QMenu* menu, const QWebHitTestResult &hitTest, const QPoint &pos)
{
    if (!m_actionsHaveImages) {
        m_actionsHaveImages = true;

        pageAction(QWebPage::Cut)->setIcon(QIcon::fromTheme("edit-cut"));
        pageAction(QWebPage::Copy)->setIcon(QIcon::fromTheme("edit-copy"));
        pageAction(QWebPage::Paste)->setIcon(QIcon::fromTheme("edit-paste"));
        pageAction(QWebPage::SelectAll)->setIcon(QIcon::fromTheme("edit-select-all"));
    }

    if (!hitTest.linkUrl().isEmpty() && hitTest.linkUrl().scheme() != "javascript") {
        createLinkContextMenu(menu, hitTest);
    }

    if (!hitTest.imageUrl().isEmpty()) {
        createImageContextMenu(menu, hitTest);
    }

    if (isMediaElement(hitTest.element())) {
        createMediaContextMenu(menu, hitTest);
    }

    QWebElement element = hitTest.element();
    if (!element.isNull() && (element.tagName().toLower() == "input" || element.tagName().toLower() == "textarea")) {
        if (menu->isEmpty()) {
            QMenu* pageMenu = page()->createStandardContextMenu();

            int i = 0;
            foreach(QAction * act, pageMenu->actions()) {
                if (act->isSeparator()) {
                    menu->addSeparator();
                    continue;
                }

                // Hiding double Direction menu (bug in QtWebKit 2.2)
                if (i == 0 && act->menu()) {
                    act->setVisible(false);
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
    }

    if (!selectedText().isEmpty()) {
        createSelectedTextContextMenu(menu, hitTest);
    }

    if (menu->isEmpty()) {
        createPageContextMenu(menu, pos);
    }

#if (QTWEBKIT_VERSION >= QTWEBKIT_VERSION_CHECK(2, 2, 0))
//    still bugged? in 4.8 RC (it shows selection of webkit's internal source, not html from page)
//    it may or may not be bug, but this implementation is useless for us
//
//    if (!selectedHtml().isEmpty())
//        menu->addAction(tr("Show source of selection"), this, SLOT(showSourceOfSelection()));
#endif

//    mApp->plugins()->populateWebViewMenu(m_menu, this, hitTest);
}

void WebView::createPageContextMenu(QMenu* menu, const QPoint &pos)
{
    QWebFrame* frameAtPos = page()->frameAt(pos);

    QAction* action = menu->addAction(tr("&Back"), this, SLOT(back()));
    action->setIcon(IconProvider::standardIcon(QStyle::SP_ArrowBack));
    action->setEnabled(history()->canGoBack());

    action = menu->addAction(tr("&Forward"), this, SLOT(forward()));
    action->setIcon(IconProvider::standardIcon(QStyle::SP_ArrowForward));
    action->setEnabled(history()->canGoForward());

    menu->addAction(IconProvider::standardIcon(QStyle::SP_BrowserReload), tr("&Reload"), this, SLOT(reload()));
    action = menu->addAction(IconProvider::standardIcon(QStyle::SP_BrowserStop), tr("S&top"), this, SLOT(stop()));
    action->setEnabled(isLoading());
    menu->addSeparator();

    if (frameAtPos && page()->mainFrame() != frameAtPos) {
        m_clickedFrame = frameAtPos;
        QMenu* frameMenu = new QMenu(tr("This frame"));
        frameMenu->addAction(tr("Show &only this frame"), this, SLOT(loadClickedFrame()));
        frameMenu->addAction(QIcon(":/icons/menu/popup.png"), tr("Show this frame in new &tab"), this, SLOT(loadClickedFrameInNewTab()));
        frameMenu->addSeparator();
        frameMenu->addAction(IconProvider::standardIcon(QStyle::SP_BrowserReload), tr("&Reload"), this, SLOT(reloadClickedFrame()));
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
    menu->addAction(IconProvider::fromTheme("user-bookmarks"), tr("Book&mark page"), this, SLOT(bookmarkLink()));
    menu->addAction(QIcon::fromTheme("document-save"), tr("&Save page as..."), this, SLOT(downloadLinkToDisk()))->setData(url());
    menu->addAction(QIcon::fromTheme("edit-copy"), tr("&Copy page link"), this, SLOT(copyLinkToClipboard()))->setData(url());
    menu->addAction(QIcon::fromTheme("mail-message-new"), tr("Send page link..."), this, SLOT(sendLinkByMail()))->setData(url());
    menu->addAction(QIcon::fromTheme("document-print"), tr("&Print page"), this, SLOT(printPage()));
    menu->addSeparator();
    menu->addAction(QIcon::fromTheme("edit-select-all"), tr("Select &all"), this, SLOT(selectAll()));
    menu->addSeparator();
    if (url().scheme() == "http" || url().scheme() == "https") {
//             bool result = validateConfirm(tr("Do you want to upload this page to an online source code validator?"));
//                 if (result)
        menu->addAction(tr("Validate page"), this, SLOT(openUrlInSelectedTab()))->setData("http://validator.w3.org/check?uri=" + url().toString());
    }

    menu->addAction(QIcon::fromTheme("text-html"), tr("Show so&urce code"), this, SLOT(showSource()));
    menu->addAction(QIcon::fromTheme("dialog-information"), tr("Show info ab&out site"), this, SLOT(showSiteInfo()));
}

void WebView::createLinkContextMenu(QMenu* menu, const QWebHitTestResult &hitTest)
{
    // Workaround for QtWebKit <= 2.0 when selecting link text on right click
    if (page()->selectedText() == hitTest.linkText()) {
        findText("");
    }

    menu->addSeparator();
    menu->addAction(QIcon(":/icons/menu/popup.png"), tr("Open link in new &tab"), this, SLOT(openUrlInBackgroundTab()))->setData(hitTest.linkUrl());
    menu->addAction(QIcon::fromTheme("window-new"), tr("Open link in new &window"), this, SLOT(openUrlInNewWindow()))->setData(hitTest.linkUrl());
    menu->addSeparator();
    menu->addAction(IconProvider::fromTheme("user-bookmarks"), tr("B&ookmark link"), this, SLOT(bookmarkLink()))->setData(hitTest.linkUrl());
    menu->addAction(QIcon::fromTheme("document-save"), tr("&Save link as..."), this, SLOT(downloadLinkToDisk()))->setData(hitTest.linkUrl());
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
    menu->addAction(tr("Show i&mage"), this, SLOT(openActionUrl()))->setData(hitTest.imageUrl());
    menu->addAction(tr("Copy im&age"), this, SLOT(copyImageToClipboard()))->setData(hitTest.imageUrl());
    menu->addAction(QIcon::fromTheme("edit-copy"), tr("Copy image ad&dress"), this, SLOT(copyLinkToClipboard()))->setData(hitTest.imageUrl());
    menu->addSeparator();
    menu->addAction(QIcon::fromTheme("document-save"), tr("&Save image as..."), this, SLOT(downloadLinkToDisk()))->setData(hitTest.imageUrl());
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

    QString langCode = mApp->getActiveLanguage().left(2);
    QUrl googleTranslateUrl = QUrl(QString("http://translate.google.com/#auto|%1|%2").arg(langCode, selectedText));
    menu->addAction(QIcon(":icons/menu/translate.png"), tr("Google Translate"), this, SLOT(openUrlInSelectedTab()))->setData(googleTranslateUrl);
    menu->addAction(QIcon::fromTheme("accessories-dictionary"), tr("Dictionary"), this, SLOT(openUrlInSelectedTab()))->setData("http://" + (langCode != "" ? langCode + "." : langCode) + "wiktionary.org/wiki/Special:Search?search=" + selectedText);
    menu->addSeparator();

    QString selectedString = selectedText.trimmed();
    if (!selectedString.contains(".")) {
        // Try to add .com
        selectedString.append(".com");
    }
    QUrl guessedUrl = QUrl::fromUserInput(selectedString);

    if (isUrlValid(guessedUrl)) {
        menu->addAction(QIcon(":/icons/menu/popup.png"), tr("Go to &web address"), this, SLOT(openUrlInSelectedTab()))->setData(guessedUrl);
    }

    selectedText.truncate(20);

    SearchEngine engine = mApp->searchEnginesManager()->activeEngine();
    menu->addAction(engine.icon, tr("Search \"%1 ..\" with %2").arg(selectedText, engine.name), this, SLOT(searchSelectedText()));
}

void WebView::createMediaContextMenu(QMenu* menu, const QWebHitTestResult &hitTest)
{
    m_mediaElement = hitTest.element();

    if (m_mediaElement.isNull()) {
        return;
    }

    bool paused = m_mediaElement.evaluateJavaScript("this.paused").toBool();
    bool muted = m_mediaElement.evaluateJavaScript("this.muted").toBool();
    QUrl videoUrl = m_mediaElement.evaluateJavaScript("this.currentSrc").toUrl();

    menu->addSeparator();
    menu->addAction(paused ? tr("&Play") : tr("&Pause"), this, SLOT(pauseMedia()))->setIcon(QIcon::fromTheme(paused ? "media-playback-start" : "media-playback-pause"));
    menu->addAction(muted ? tr("Un&mute") : tr("&Mute"), this, SLOT(muteMedia()))->setIcon(QIcon::fromTheme(muted ? "audio-volume-muted" : "audio-volume-high"));
    menu->addSeparator();
    menu->addAction(QIcon::fromTheme("edit-copy"), tr("&Copy Media Address"), this, SLOT(copyLinkToClipboard()))->setData(videoUrl);
    menu->addAction(QIcon::fromTheme("mail-message-new"), tr("&Send Media Address"), this, SLOT(sendLinkByMail()))->setData(videoUrl);
    menu->addAction(QIcon::fromTheme("document-save"), tr("Save Media To &Disk"), this, SLOT(downloadLinkToDisk()))->setData(videoUrl);
}

void WebView::pauseMedia()
{
    bool paused = m_mediaElement.evaluateJavaScript("this.paused").toBool();

    if (paused) {
        m_mediaElement.evaluateJavaScript("this.play()");
    }
    else {
        m_mediaElement.evaluateJavaScript("this.pause()");
    }
}

void WebView::muteMedia()
{
    bool muted = m_mediaElement.evaluateJavaScript("this.muted").toBool();

    if (muted) {
        m_mediaElement.evaluateJavaScript("this.muted = false");
    }
    else {
        m_mediaElement.evaluateJavaScript("this.muted = true");
    }
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

    QWebView::wheelEvent(event);
}

void WebView::mousePressEvent(QMouseEvent* event)
{
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
#ifdef Q_WS_WIN
        QWebFrame* frame = page()->frameAt(event->pos());
        if (frame && frame->hitTestContent(event->pos()).linkUrl().isEmpty()) {
            // Creating auto scroll on Windows
            m_clickedUrl = QUrl();
            QWebView::mouseDoubleClickEvent(event);
            return;
        }
#endif
    }

    case Qt::LeftButton: {
        QWebFrame* frame = page()->frameAt(event->pos());
        if (frame) {
            m_clickedUrl = frame->hitTestContent(event->pos()).linkUrl();
        }
    }

    default:
        break;
    }

    QWebView::mousePressEvent(event);
}

void WebView::mouseReleaseEvent(QMouseEvent* event)
{
    switch (event->button()) {
    case Qt::MiddleButton: {
        QWebFrame* frame = page()->frameAt(event->pos());
        if (frame) {
            QUrl link = frame->hitTestContent(event->pos()).linkUrl();
            if (m_clickedUrl == link && isUrlValid(link)) {
                openUrlInNewTab(link, Qz::NT_NotSelectedTab);
                event->accept();
                return;
            }
        }

        break;
    }

    case Qt::LeftButton: {
        QWebFrame* frame = page()->frameAt(event->pos());
        if (frame) {
            QUrl link = frame->hitTestContent(event->pos()).linkUrl();
            if (m_clickedUrl == link && event->modifiers() == Qt::ControlModifier) {
                if (isUrlValid(link)) {
                    openUrlInNewTab(link, WebViewSettings::newTabAfterActive ? Qz::NT_NotSelectedTab : Qz::NT_CleanTab);
                    event->accept();
                    return;
                }
            }
        }

        break;
    }

    default:
        break;
    }

    QWebView::mouseReleaseEvent(event);
}

void WebView::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
    case Qt::Key_C:
        if (event->modifiers() == Qt::ControlModifier) {
            copyText();
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
    if (obj != this) {
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

        if (ev->type() == QEvent::MouseButtonPress && ev->buttons() & Qt::RightButton) {
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

