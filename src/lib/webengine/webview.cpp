/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
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
#include "pluginproxy.h"
#include "downloadmanager.h"
#include "sourceviewer.h"
#include "siteinfo.h"
#include "searchenginesmanager.h"
#include "browsinglibrary.h"
#include "bookmarkstools.h"
#include "settings.h"
#include "qzsettings.h"
#include "enhancedmenu.h"
#include "locationbar.h"
#include "webinspector.h"
#include "scripts.h"

#ifdef USE_HUNSPELL
#include "qtwebkit/spellcheck/speller.h"
#endif

#include <iostream>

#include <QDir>
#include <QTimer>
#include <QDesktopServices>
#include <QNetworkRequest>
#include <QWebEngineHistory>
#include <QClipboard>

bool WebView::s_forceContextMenuOnMouseRelease = false;

WebView::WebView(QWidget* parent)
    : QWebEngineView(parent)
    , m_siteIconLoader(0)
    , m_progress(100)
    , m_page(0)
    , m_firstLoad(false)
{
    connect(this, SIGNAL(loadStarted()), this, SLOT(slotLoadStarted()));
    connect(this, SIGNAL(loadProgress(int)), this, SLOT(slotLoadProgress(int)));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(slotLoadFinished()));
    connect(this, SIGNAL(iconUrlChanged(QUrl)), this, SLOT(slotIconUrlChanged(QUrl)));

    m_currentZoomLevel = zoomLevels().indexOf(100);

    installEventFilter(this);

    WebInspector::registerView(this);
}

WebView::~WebView()
{
    WebInspector::unregisterView(this);
}

QIcon WebView::icon() const
{
    if (url().scheme() == QLatin1String("qupzilla")) {
        return QIcon(":icons/qupzilla.png");
    }

    if (url().scheme() == QLatin1String("file")) {
        return IconProvider::standardIcon(QStyle::SP_DriveHDIcon);
    }

    if (url().scheme() == QLatin1String("ftp")) {
        return IconProvider::standardIcon(QStyle::SP_ComputerIcon);
    }

    if (!m_siteIcon.isNull() && m_siteIconUrl.host() == url().host()) {
        return m_siteIcon;
    }

    return IconProvider::iconForUrl(url());
}

QString WebView::title() const
{
    QString title = QWebEngineView::title();

    if (title.isEmpty()) {
        title = url().toString(QUrl::RemoveFragment);
    }

    if (title.isEmpty() || title == QLatin1String("about:blank")) {
        return tr("Empty Page");
    }

    return title;
}

bool WebView::isTitleEmpty() const
{
    return QWebEngineView::title().isEmpty();
}

WebPage* WebView::page() const
{
    return m_page;
}

void WebView::setPage(WebPage *page)
{
    if (m_page == page) {
        return;
    }

    QWebEngineView::setPage(page);
    m_page = qobject_cast<WebPage*>(page);

    connect(m_page, SIGNAL(privacyChanged(bool)), this, SIGNAL(privacyChanged(bool)));

    // Set default zoom level
    zoomReset();

    // Actions needs to be initialized for every QWebPage change
    initializeActions();

    mApp->plugins()->emitWebPageCreated(m_page);
}

void WebView::load(const QUrl &url)
{
    QWebEngineView::load(url);

    if (!m_firstLoad) {
        m_firstLoad = true;
        WebInspector::pushView(this);
    }
}

void WebView::load(const LoadRequest &request)
{
    const QUrl reqUrl = request.url();

    if (reqUrl.scheme() == QL1S("javascript")) {
        const QString scriptSource = reqUrl.toString().mid(11);
        // Is the javascript source percent encoded or not?
        // Looking for % character in source should work in most cases
        if (scriptSource.contains(QL1C('%')))
            page()->runJavaScript(QUrl::fromPercentEncoding(scriptSource.toUtf8()));
        else
            page()->runJavaScript(scriptSource);
        return;
    }

    if (reqUrl.isEmpty() || isUrlValid(reqUrl)) {
        loadRequest(request);
        return;
    }

    // Make sure to correctly load hosts like localhost (eg. without the dot)
    if (!reqUrl.isEmpty() &&
        reqUrl.scheme().isEmpty() &&
        !reqUrl.path().contains(QL1C(' ')) &&
        !reqUrl.path().contains(QL1C('.'))
       ) {
        LoadRequest req = request;
        req.setUrl(QUrl(QSL("http://") + reqUrl.path()));
        loadRequest(req);
        return;
    }

    const LoadRequest searchRequest = mApp->searchEnginesManager()->searchResult(request.urlString());
    loadRequest(searchRequest);
}

bool WebView::isLoading() const
{
    return m_progress < 100;
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

int WebView::zoomLevel() const
{
    return m_currentZoomLevel;
}

void WebView::setZoomLevel(int level)
{
    m_currentZoomLevel = level;
    applyZoom();
}

void WebView::restoreHistory(const QByteArray &data)
{
    QDataStream stream(data);
    stream >> *history();

    // Workaround clearing QWebChannel after restoring history
    m_page->setupWebChannel();
}

bool WebView::onBeforeUnload()
{
#if QTWEBENGINE_DISABLED
    const QString res = page()->mainFrame()->evaluateJavaScript("window.onbeforeunload(new Event(\"beforeunload\"))").toString();

    if (!res.isEmpty()) {
        return page()->javaScriptConfirm(page()->mainFrame(), res);
    }
#endif

    return true;
}

// static
bool WebView::isUrlValid(const QUrl &url)
{
    // Valid url must have scheme and actually contains something (therefore scheme:// is invalid)
    return url.isValid() && !url.scheme().isEmpty() && (!url.host().isEmpty() || !url.path().isEmpty() || url.hasQuery());
}

// static
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

// static
QList<int> WebView::zoomLevels()
{
    return QList<int>() << 30 << 40 << 50 << 67 << 80 << 90 << 100
           << 110 << 120 << 133 << 150 << 170 << 200
           << 220 << 233 << 250 << 270 << 285 << 300;
}

// static
bool WebView::forceContextMenuOnMouseRelease()
{
    return s_forceContextMenuOnMouseRelease;
}

// static
void WebView::setForceContextMenuOnMouseRelease(bool force)
{
    s_forceContextMenuOnMouseRelease = force;
}

void WebView::addNotification(QWidget* notif)
{
    emit showNotification(notif);
}

void WebView::applyZoom()
{
    setZoomFactor(qreal(zoomLevels().at(m_currentZoomLevel)) / 100.0);

    emit zoomLevelChanged(m_currentZoomLevel);
}

void WebView::zoomIn()
{
    if (m_currentZoomLevel < zoomLevels().count() - 1) {
        m_currentZoomLevel++;
        applyZoom();
    }
}

void WebView::zoomOut()
{
    if (m_currentZoomLevel > 0) {
        m_currentZoomLevel--;
        applyZoom();
    }
}

void WebView::zoomReset()
{
    if (m_currentZoomLevel != qzSettings->defaultZoomLevel) {
        m_currentZoomLevel = qzSettings->defaultZoomLevel;
        applyZoom();
    }
}

void WebView::editUndo()
{
    triggerPageAction(QWebEnginePage::Undo);
}

void WebView::editRedo()
{
    triggerPageAction(QWebEnginePage::Redo);
}

void WebView::editCut()
{
    triggerPageAction(QWebEnginePage::Cut);
}

void WebView::editCopy()
{
    triggerPageAction(QWebEnginePage::Copy);
}

void WebView::editPaste()
{
    triggerPageAction(QWebEnginePage::Paste);
}

void WebView::editSelectAll()
{
    triggerPageAction(QWebEnginePage::SelectAll);
}

void WebView::editDelete()
{
    QKeyEvent ev(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
    QApplication::sendEvent(this, &ev);
}

void WebView::reloadBypassCache()
{
    triggerPageAction(QWebEnginePage::ReloadAndBypassCache);
}

void WebView::back()
{
    QWebEngineHistory* history = page()->history();

    if (history->canGoBack()) {
        history->back();

        emit urlChanged(url());
    }
}

void WebView::forward()
{
    QWebEngineHistory* history = page()->history();

    if (history->canGoForward()) {
        history->forward();

        emit urlChanged(url());
    }
}

void WebView::slotLoadStarted()
{
    m_progress = 0;
}

void WebView::slotLoadProgress(int progress)
{
    m_progress = progress;
}

void WebView::slotLoadFinished()
{
    m_progress = 100;

    mApp->history()->addHistoryEntry(this);
}

void WebView::slotIconUrlChanged(const QUrl &url)
{
    if (m_siteIconUrl == url) {
        emit iconChanged();
        return;
    }

    delete m_siteIconLoader;
    m_siteIconLoader = new IconLoader(url, this);

    connect(m_siteIconLoader, &IconLoader::iconLoaded, [this, url](const QIcon &icon) {
        if (icon.isNull())
            return;

        m_siteIcon = icon;
        m_siteIconUrl = url;
        emit iconChanged();

        IconProvider::instance()->saveIcon(this);
    });
}

void WebView::openUrlInNewWindow()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        mApp->createWindow(Qz::BW_NewWindow, action->data().toUrl());
    }
}

void WebView::sendLinkByMail()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        const QUrl mailUrl = QUrl::fromEncoded("mailto:%20?body=" + QUrl::toPercentEncoding(action->data().toUrl().toEncoded()));
        QDesktopServices::openUrl(mailUrl);
    }
}

void WebView::sendPageByMail()
{
    const QUrl mailUrl = QUrl::fromEncoded("mailto:%20?body=" + QUrl::toPercentEncoding(url().toEncoded()) + "&subject=" + QUrl::toPercentEncoding(title()));
    QDesktopServices::openUrl(mailUrl);
}

void WebView::copyLinkToClipboard()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        QApplication::clipboard()->setText(action->data().toUrl().toEncoded());
    }
}

void WebView::savePageAs()
{
#if QTWEBENGINE_DISABLED
    if (url().isEmpty() || url().toString() == QLatin1String("about:blank")) {
        return;
    }

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

    DownloadManager* dManager = mApp->downloadManager();
    dManager->download(request, info);
#endif
}

void WebView::openUrlInNewTab(const QUrl &url, Qz::NewTabPositionFlags position)
{
    QNetworkRequest request(url);
    request.setRawHeader("X-QupZilla-UserLoadAction", QByteArray("1"));

    loadInNewTab(request, position);
}

void WebView::downloadUrlToDisk()
{
#if QTWEBENGINE_DISABLED
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        QNetworkRequest request(action->data().toUrl());

        DownloadManager::DownloadInfo info;
        info.page = page();
        info.suggestedFileName = QString();
        info.askWhatToDo = false;
        info.forceChoosingPath = true;

        DownloadManager* dManager = mApp->downloadManager();
        dManager->download(request, info);
    }
#endif
}

void WebView::copyImageToClipboard()
{
#if QTWEBENGINE_DISABLED
    triggerPageAction(QWebEnginePage::CopyImageToClipboard);
#endif
}

void WebView::openActionUrl()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        load(action->data().toUrl());
    }
}

void WebView::showSource()
{
    // view-source: doesn't work on itself and custom schemes
    if (url().scheme() == QL1S("view-source") || url().scheme() == QL1S("qupzilla") || url().scheme() == QL1S("qrc")) {
        page()->toHtml([](const QString &html) {
            std::cout << html.toLocal8Bit().constData() << std::endl;
        });
        return;
    }

    QUrl u;
    u.setScheme(QSL("view-source"));
    u.setPath(url().toString());
    openUrlInNewTab(u, Qz::NT_SelectedTab);
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

    LoadRequest req = mApp->searchEnginesManager()->searchResult(engine, selectedText());
    QNetworkRequest r = req.networkRequest();
    r.setRawHeader("Referer", req.url().toEncoded());
    r.setRawHeader("X-QupZilla-UserLoadAction", QByteArray("1"));
    req.setNetworkRequest(r);

    loadInNewTab(req, Qz::NT_SelectedTab);
}

void WebView::searchSelectedTextInBackgroundTab()
{
    SearchEngine engine = mApp->searchEnginesManager()->activeEngine();
    if (QAction* act = qobject_cast<QAction*>(sender())) {
        if (act->data().isValid()) {
            engine = act->data().value<SearchEngine>();
        }
    }

    LoadRequest req = mApp->searchEnginesManager()->searchResult(engine, selectedText());
    QNetworkRequest r = req.networkRequest();
    r.setRawHeader("Referer", req.url().toEncoded());
    r.setRawHeader("X-QupZilla-UserLoadAction", QByteArray("1"));
    req.setNetworkRequest(r);

    loadInNewTab(req, Qz::NT_NotSelectedTab);
}

void WebView::bookmarkLink()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        if (action->data().isNull()) {
            BookmarksTools::addBookmarkDialog(this, url(), title());
        }
        else {
            const QVariantList bData = action->data().value<QVariantList>();
            const QString bookmarkTitle = bData.at(1).toString().isEmpty() ? title() : bData.at(1).toString();

            BookmarksTools::addBookmarkDialog(this, bData.at(0).toUrl(), bookmarkTitle);
        }
    }
}

void WebView::openUrlInSelectedTab()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        openUrlInNewTab(action->data().toUrl(), Qz::NT_CleanSelectedTab);
    }
}

void WebView::openUrlInBackgroundTab()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        openUrlInNewTab(action->data().toUrl(), Qz::NT_CleanNotSelectedTab);
    }
}

void WebView::userDefinedOpenUrlInNewTab(const QUrl &url, bool invert)
{
    Qz::NewTabPositionFlags position = qzSettings->newTabPosition;
    if (invert) {
        if (position & Qz::NT_SelectedTab) {
            position &= ~Qz::NT_SelectedTab;
            position |= Qz::NT_NotSelectedTab;
        }
        else {
            position &= ~Qz::NT_NotSelectedTab;
            position |= Qz::NT_SelectedTab;
        }
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

#if QTWEBENGINE_DISABLED
void WebView::printPage(QWebEngineFrame* frame)
{
    QPrintPreviewDialog* dialog = new QPrintPreviewDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->resize(800, 750);

    if (!frame) {
        connect(dialog, SIGNAL(paintRequested(QPrinter*)), this, SLOT(print(QPrinter*)));
    }
    else {
        connect(dialog, SIGNAL(paintRequested(QPrinter*)), frame, SLOT(print(QPrinter*)));
    }

    dialog->open();
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

    const QString url = parentElement.attribute("action");
    const QString method = parentElement.hasAttribute("method") ? parentElement.attribute("method").toUpper() : "GET";

    if (!url.isEmpty() && (method == QLatin1String("GET") || method == QLatin1String("POST"))) {
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
    // cppcheck-suppress variableScope
    int spellCheckActionCount = 0;

#ifdef USE_HUNSPELL
    // Show spellcheck menu as the first
    if (hitTest.isContentEditable() && !hitTest.isContentSelected()) {
        Speller::instance()->populateContextMenu(menu, hitTest);
        spellCheckActionCount = menu->actions().count();
    }
#endif

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
        // This only checks if the menu is empty (only spellchecker actions added)
        if (menu->actions().count() == spellCheckActionCount) {
            QMenu* pageMenu = page()->createStandardContextMenu();
            // Apparently createStandardContextMenu() can return null pointer
            if (pageMenu) {
                if (qzSettings->enableFormsUndoRedo) {
                    menu->addAction(pageAction(QWebPage::Undo));
                    menu->addAction(pageAction(QWebPage::Redo));
                    menu->addSeparator();
                }
                int i = 0;
                foreach (QAction* act, pageMenu->actions()) {
                    if (act->isSeparator()) {
                        menu->addSeparator();
                        continue;
                    }

                    // Hiding double Direction + Fonts menu (bug in QtWebKit 2.2)
                    if (i <= 1 && act->menu()) {
                        if (act->menu()->actions().contains(pageAction(QWebPage::SetTextDirectionDefault)) ||
                            act->menu()->actions().contains(pageAction(QWebPage::ToggleBold))
                           ) {
                            act->setVisible(false);
                        }
                    }

                    menu->addAction(act);

                    if (act == pageAction(QWebPage::Paste)) {
                        QAction* a = menu->addAction(QIcon::fromTheme("edit-delete"), tr("Delete"), this, SLOT(editDelete()));
                        a->setShortcut(QKeySequence("Del"));
                    }

                    ++i;
                }

                if (menu->actions().last() == pageAction(QWebEnginePage::InspectElement)) {
                    // We have own Inspect Element action
                    menu->actions().last()->setVisible(false);
                }

                delete pageMenu;
            }
        }

        if (hitTest.element().tagName().toLower() == QLatin1String("input")) {
            checkForForm(menu, hitTest.element());
        }

#ifdef USE_HUNSPELL
        Speller::instance()->createContextMenu(menu);
#endif
    }

    if (!selectedText().isEmpty()) {
        createSelectedTextContextMenu(menu, hitTest);
    }

    if (menu->isEmpty()) {
        createPageContextMenu(menu, pos);
    }

    menu->addSeparator();
    mApp->plugins()->populateWebViewMenu(menu, this, hitTest);
}

void WebView::createPageContextMenu(QMenu* menu, const QPoint &pos)
{
    QWebEngineFrame* frameAtPos = page()->frameAt(pos);

    QAction* action = menu->addAction(tr("&Back"), this, SLOT(back()));
    action->setIcon(IconProvider::standardIcon(QStyle::SP_ArrowBack));
    action->setEnabled(history()->canGoBack());

    action = menu->addAction(tr("&Forward"), this, SLOT(forward()));
    action->setIcon(IconProvider::standardIcon(QStyle::SP_ArrowForward));
    action->setEnabled(history()->canGoForward());

    // Special menu for Speed Dial page
    if (url().toString() == QL1S("qupzilla:speeddial")) {
        menu->addSeparator();
        menu->addAction(QIcon::fromTheme("list-add"), tr("&Add New Page"), this, SLOT(addSpeedDial()));
        menu->addAction(IconProvider::settingsIcon(), tr("&Configure Speed Dial"), this, SLOT(configureSpeedDial()));
        menu->addSeparator();
        menu->addAction(QIcon::fromTheme(QSL("view-refresh")), tr("Reload All Dials"), this, SLOT(reloadAllSpeedDials()));
        return;
    }

    menu->addAction(pageAction(QWebPage::Reload));
    menu->addAction(pageAction(QWebPage::Stop));
    menu->addSeparator();

    if (frameAtPos && page()->mainFrame() != frameAtPos) {
        m_clickedFrame = frameAtPos;
        Menu* frameMenu = new Menu(tr("This frame"));
        frameMenu->setCloseOnMiddleClick(true);
        frameMenu->addAction(tr("Show &only this frame"), this, SLOT(loadClickedFrame()));
        Action* act = new Action(IconProvider::newTabIcon(), tr("Show this frame in new &tab"));
        connect(act, SIGNAL(triggered()), this, SLOT(loadClickedFrameInNewTab()));
        connect(act, SIGNAL(ctrlTriggered()), this, SLOT(loadClickedFrameInBgTab()));
        frameMenu->addAction(act);
        frameMenu->addSeparator();
        frameMenu->addAction(QIcon::fromTheme(QSL("view-refresh")), tr("&Reload"), this, SLOT(reloadClickedFrame()));
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
    menu->addAction(QIcon::fromTheme("bookmark-new"), tr("Book&mark page"), this, SLOT(bookmarkLink()));
    menu->addAction(QIcon::fromTheme("document-save"), tr("&Save page as..."), this, SLOT(savePageAs()));
    menu->addAction(QIcon::fromTheme("edit-copy"), tr("&Copy page link"), this, SLOT(copyLinkToClipboard()))->setData(url());
    menu->addAction(QIcon::fromTheme("mail-message-new"), tr("Send page link..."), this, SLOT(sendPageByMail()));
    menu->addAction(QIcon::fromTheme("document-print"), tr("&Print page"), this, SLOT(printPage()));
    menu->addSeparator();
    menu->addAction(QIcon::fromTheme("edit-select-all"), tr("Select &all"), this, SLOT(editSelectAll()));
    menu->addSeparator();

    if (url().scheme() == QLatin1String("http") || url().scheme() == QLatin1String("https")) {
        const QUrl w3url = QUrl::fromEncoded("http://validator.w3.org/check?uri=" + QUrl::toPercentEncoding(url().toEncoded()));
        menu->addAction(QIcon(":icons/sites/w3.png"), tr("Validate page"), this, SLOT(openUrlInSelectedTab()))->setData(w3url);

        QByteArray langCode = mApp->currentLanguage().left(2).toUtf8();
        const QUrl gturl = QUrl::fromEncoded("http://translate.google.com/translate?sl=auto&tl=" + langCode + "&u=" + QUrl::toPercentEncoding(url().toEncoded()));
        menu->addAction(QIcon(":icons/sites/translate.png"), tr("Translate page"), this, SLOT(openUrlInSelectedTab()))->setData(gturl);
    }

    menu->addSeparator();
    menu->addAction(QIcon::fromTheme("text-html"), tr("Show so&urce code"), this, SLOT(showSource()));
    menu->addAction(QIcon::fromTheme("dialog-information"), tr("Show info ab&out site"), this, SLOT(showSiteInfo()));
}

void WebView::createLinkContextMenu(QMenu* menu, const QWebHitTestResult &hitTest)
{
    menu->addSeparator();
    Action* act = new Action(IconProvider::newTabIcon(), tr("Open link in new &tab"));
    act->setData(hitTest.linkUrl());
    connect(act, SIGNAL(triggered()), this, SLOT(userDefinedOpenUrlInNewTab()));
    connect(act, SIGNAL(ctrlTriggered()), this, SLOT(userDefinedOpenUrlInBgTab()));
    menu->addAction(act);
    menu->addAction(IconProvider::newWindowIcon(), tr("Open link in new &window"), this, SLOT(openUrlInNewWindow()))->setData(hitTest.linkUrl());
    menu->addAction(IconProvider::privateBrowsingIcon(), tr("Open link in &private window"), mApp, SLOT(startPrivateBrowsing()))->setData(hitTest.linkUrl());
    menu->addSeparator();

    QVariantList bData;
    bData << hitTest.linkUrl() << hitTest.linkTitle();
    menu->addAction(QIcon::fromTheme("bookmark-new"), tr("B&ookmark link"), this, SLOT(bookmarkLink()))->setData(bData);

    menu->addAction(QIcon::fromTheme("document-save"), tr("&Save link as..."), this, SLOT(downloadUrlToDisk()))->setData(hitTest.linkUrl());
    menu->addAction(QIcon::fromTheme("mail-message-new"), tr("Send link..."), this, SLOT(sendLinkByMail()))->setData(hitTest.linkUrl());
    menu->addAction(QIcon::fromTheme("edit-copy"), tr("&Copy link address"), this, SLOT(copyLinkToClipboard()))->setData(hitTest.linkUrl());
    menu->addSeparator();

    if (!selectedText().isEmpty()) {
        pageAction(QWebEnginePage::Copy)->setIcon(QIcon::fromTheme("edit-copy"));
        menu->addAction(pageAction(QWebEnginePage::Copy));
    }
}

void WebView::createImageContextMenu(QMenu* menu, const QWebHitTestResult &hitTest)
{
    menu->addSeparator();
    Action* act = new Action(tr("Show i&mage"));
    act->setData(hitTest.imageUrl());
    connect(act, SIGNAL(triggered()), this, SLOT(openActionUrl()));
    connect(act, SIGNAL(ctrlTriggered()), this, SLOT(userDefinedOpenUrlInNewTab()));
    menu->addAction(act);
    menu->addAction(tr("Copy im&age"), this, SLOT(copyImageToClipboard()))->setData(hitTest.imageUrl());
    menu->addAction(QIcon::fromTheme("edit-copy"), tr("Copy image ad&dress"), this, SLOT(copyLinkToClipboard()))->setData(hitTest.imageUrl());
    menu->addSeparator();
    menu->addAction(QIcon::fromTheme("document-save"), tr("&Save image as..."), this, SLOT(downloadUrlToDisk()))->setData(hitTest.imageUrl());
    menu->addAction(QIcon::fromTheme("mail-message-new"), tr("Send image..."), this, SLOT(sendLinkByMail()))->setData(hitTest.imageUrl());
    menu->addSeparator();

    if (!selectedText().isEmpty()) {
        pageAction(QWebEnginePage::Copy)->setIcon(QIcon::fromTheme("edit-copy"));
        menu->addAction(pageAction(QWebEnginePage::Copy));
    }
}

void WebView::createSelectedTextContextMenu(QMenu* menu, const QWebHitTestResult &hitTest)
{
    Q_UNUSED(hitTest)

    QString selectedText = page()->selectedText();

    menu->addSeparator();
    if (!menu->actions().contains(pageAction(QWebEnginePage::Copy))) {
        menu->addAction(pageAction(QWebEnginePage::Copy));
    }
    menu->addAction(QIcon::fromTheme("mail-message-new"), tr("Send text..."), this, SLOT(sendLinkByMail()))->setData(selectedText);
    menu->addSeparator();

    QString langCode = mApp->currentLanguage().left(2).toUtf8();
    QUrl googleTranslateUrl = QUrl(QString("https://translate.google.com/#auto/%1/%2").arg(langCode, selectedText));
    Action* gtwact = new Action(QIcon(":icons/sites/translate.png"), tr("Google Translate"));
    gtwact->setData(googleTranslateUrl);
    connect(gtwact, SIGNAL(triggered()), this, SLOT(openUrlInSelectedTab()));
    connect(gtwact, SIGNAL(ctrlTriggered()), this, SLOT(openUrlInBackgroundTab()));
    menu->addAction(gtwact);

    Action* dictact = new Action(QIcon::fromTheme("accessories-dictionary"), tr("Dictionary"));
    dictact->setData(QUrl("http://" + (!langCode.isEmpty() ? langCode + "." : langCode) + "wiktionary.org/wiki/Special:Search?search=" + selectedText));
    connect(dictact, SIGNAL(triggered()), this, SLOT(openUrlInSelectedTab()));
    connect(dictact, SIGNAL(ctrlTriggered()), this, SLOT(openUrlInBackgroundTab()));
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
        connect(act, SIGNAL(ctrlTriggered()), this, SLOT(userDefinedOpenUrlInNewTab()));
        menu->addAction(act);
    }

    menu->addSeparator();
    selectedText.truncate(20);
    // KDE is displaying newlines in menu actions ... weird -,-
    selectedText.replace(QLatin1Char('\n'), QLatin1Char(' ')).replace(QLatin1Char('\t'), QLatin1Char(' '));

    SearchEngine engine = mApp->searchEnginesManager()->activeEngine();
    Action* act = new Action(engine.icon, tr("Search \"%1 ..\" with %2").arg(selectedText, engine.name));
    connect(act, SIGNAL(triggered()), this, SLOT(searchSelectedText()));
    connect(act, SIGNAL(ctrlTriggered()), this, SLOT(searchSelectedTextInBackgroundTab()));
    menu->addAction(act);

    // Search with ...
    Menu* swMenu = new Menu(tr("Search with..."), menu);
    swMenu->setCloseOnMiddleClick(true);
    SearchEnginesManager* searchManager = mApp->searchEnginesManager();
    foreach (const SearchEngine &en, searchManager->allEngines()) {
        Action* act = new Action(en.icon, en.name);
        act->setData(QVariant::fromValue(en));

        connect(act, SIGNAL(triggered()), this, SLOT(searchSelectedText()));
        connect(act, SIGNAL(ctrlTriggered()), this, SLOT(searchSelectedTextInBackgroundTab()));
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
#endif

void WebView::addSpeedDial()
{
    page()->runJavaScript("addSpeedDial()");
}

void WebView::configureSpeedDial()
{
    page()->runJavaScript("configureSpeedDial()");
}

void WebView::reloadAllSpeedDials()
{
    page()->runJavaScript("reloadAll()");
}

void WebView::initializeActions()
{
    QAction* undoAction = pageAction(QWebEnginePage::Undo);
    undoAction->setText(tr("&Undo"));
    undoAction->setShortcut(QKeySequence("Ctrl+Z"));
    undoAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    undoAction->setIcon(QIcon::fromTheme(QSL("edit-undo")));

    QAction* redoAction = pageAction(QWebEnginePage::Redo);
    redoAction->setText(tr("&Redo"));
    redoAction->setShortcut(QKeySequence("Ctrl+Shift+Z"));
    redoAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    redoAction->setIcon(QIcon::fromTheme(QSL("edit-redo")));

    QAction* cutAction = pageAction(QWebEnginePage::Cut);
    cutAction->setText(tr("&Cut"));
    cutAction->setShortcut(QKeySequence("Ctrl+X"));
    cutAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    cutAction->setIcon(QIcon::fromTheme(QSL("edit-cut")));

    QAction* copyAction = pageAction(QWebEnginePage::Copy);
    copyAction->setText(tr("&Copy"));
    copyAction->setShortcut(QKeySequence("Ctrl+C"));
    copyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    copyAction->setIcon(QIcon::fromTheme(QSL("edit-copy")));

    QAction* pasteAction = pageAction(QWebEnginePage::Paste);
    pasteAction->setText(tr("&Paste"));
    pasteAction->setShortcut(QKeySequence("Ctrl+V"));
    pasteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    pasteAction->setIcon(QIcon::fromTheme(QSL("edit-paste")));

    QAction* selectAllAction = pageAction(QWebEnginePage::SelectAll);
    selectAllAction->setText(tr("Select All"));
    selectAllAction->setShortcut(QKeySequence("Ctrl+A"));
    selectAllAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    selectAllAction->setIcon(QIcon::fromTheme(QSL("edit-select-all")));

    QAction* reloadAction = pageAction(QWebEnginePage::Reload);
    reloadAction->setText(tr("&Reload"));
    reloadAction->setIcon(QIcon::fromTheme(QSL("view-refresh")));

    QAction* stopAction = pageAction(QWebEnginePage::Stop);
    stopAction->setText(tr("S&top"));
    stopAction->setIcon(QIcon::fromTheme(QSL("process-stop")));

    // Make action shortcuts available for webview
    addAction(undoAction);
    addAction(redoAction);
    addAction(cutAction);
    addAction(copyAction);
    addAction(pasteAction);
    addAction(selectAllAction);
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

    QWebEngineView::wheelEvent(event);
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
#if QTWEBENGINE_DISABLED
        QWebEngineFrame* frame = page()->frameAt(event->pos());
        if (frame) {
            m_clickedUrl = frame->hitTestContent(event->pos()).linkUrl();
            if (!m_clickedUrl.isEmpty()) {
                return;
            }
        }
#endif

        break;
    }

    case Qt::LeftButton: {
#if QTWEBENGINE_DISABLED
        QWebEngineFrame* frame = page()->frameAt(event->pos());
        if (frame) {
            const QUrl link = frame->hitTestContent(event->pos()).linkUrl();
            if (event->modifiers() & Qt::ControlModifier && isUrlValid(link)) {
                userDefinedOpenUrlInNewTab(link, event->modifiers() & Qt::ShiftModifier);
                event->accept();
                return;
            }
        }
#endif
    }

    default:
        break;
    }

    QWebEngineView::mousePressEvent(event);
}

void WebView::mouseReleaseEvent(QMouseEvent* event)
{
    if (mApp->plugins()->processMouseRelease(Qz::ON_WebView, this, event)) {
        return;
    }

    switch (event->button()) {
    case Qt::MiddleButton: {
#if QTWEBENGINE_DISABLED
        QWebEngineFrame* frame = page()->frameAt(event->pos());
        if (frame) {
            const QUrl link = frame->hitTestContent(event->pos()).linkUrl();
            if (m_clickedUrl == link && isUrlValid(link)) {
                userDefinedOpenUrlInNewTab(link, event->modifiers() & Qt::ShiftModifier);
                event->accept();
                return;
            }
        }
#endif
        break;
    }

    case Qt::RightButton:
        if (s_forceContextMenuOnMouseRelease) {
            QContextMenuEvent ev(QContextMenuEvent::Mouse, event->pos(), event->globalPos(), event->modifiers());
            QApplication::sendEvent(this, &ev);
        }
        break;

    default:
        break;
    }

    QWebEngineView::mouseReleaseEvent(event);
}

void WebView::mouseMoveEvent(QMouseEvent* event)
{
    if (mApp->plugins()->processMouseMove(Qz::ON_WebView, this, event)) {
        return;
    }

    QWebEngineView::mouseMoveEvent(event);
}

void WebView::keyPressEvent(QKeyEvent* event)
{
    if (mApp->plugins()->processKeyPress(Qz::ON_WebView, this, event)) {
        return;
    }

#if QTWEBENGINE_DISABLED
    int eventKey = event->key();

    switch (eventKey) {
    case Qt::Key_ZoomIn:
        zoomIn();
        event->accept();
        return;

    case Qt::Key_ZoomOut:
        zoomOut();
        event->accept();
        return;

    case Qt::Key_Plus:
        if (event->modifiers() & Qt::ControlModifier) {
            zoomIn();
            event->accept();
            return;
        }
        break;

    case Qt::Key_Minus:
        if (event->modifiers() & Qt::ControlModifier) {
            zoomOut();
            event->accept();
            return;
        }
        break;

    case Qt::Key_0:
        if (event->modifiers() & Qt::ControlModifier) {
            zoomReset();
            event->accept();
            return;
        }
        break;

    default:
        break;
    }

    // Text navigation is handled automatically in editable elements
    const QString js = QSL("document.activeElement.contentEditable==='true'||typeof document.activeElement.value != 'undefined'");
    QWebFrame* frame = page()->currentFrame();
    if (frame && frame->evaluateJavaScript(js).toBool())
        return QWebView::keyPressEvent(event);

    switch (eventKey) {
    case Qt::Key_Up:
        if (event->modifiers() & Qt::ShiftModifier) {
            triggerPageAction(QWebPage::SelectPreviousLine);
            event->accept();
            return;
        }
        break;

    case Qt::Key_Down:
        if (event->modifiers() & Qt::ShiftModifier) {
            triggerPageAction(QWebEnginePage::SelectNextLine);
            event->accept();
            return;
        }
        break;

    case Qt::Key_Left:
        if (event->modifiers() & Qt::ShiftModifier) {
            if (event->modifiers() == Qt::ShiftModifier) {
                triggerPageAction(QWebEnginePage::SelectPreviousChar);
            }
            else if (event->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier)) {
                triggerPageAction(QWebEnginePage::SelectPreviousWord);
            }
            event->accept();
            return;
        }
        break;

    case Qt::Key_Right:
        if (event->modifiers() & Qt::ShiftModifier) {
            if (event->modifiers() == Qt::ShiftModifier) {
                triggerPageAction(QWebEnginePage::SelectNextChar);
            }
            else if (event->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier)) {
                triggerPageAction(QWebEnginePage::SelectNextWord);
            }
            event->accept();
            return;
        }
        break;

    case Qt::Key_Home:
        if (event->modifiers() & Qt::ShiftModifier) {
            if (event->modifiers() == Qt::ShiftModifier) {
                triggerPageAction(QWebEnginePage::SelectStartOfLine);
            }
            else if (event->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier)) {
                triggerPageAction(QWebEnginePage::SelectStartOfDocument);
            }
            event->accept();
            return;
        }
        break;

    case Qt::Key_End:
        if (event->modifiers() & Qt::ShiftModifier) {
            if (event->modifiers() == Qt::ShiftModifier) {
                triggerPageAction(QWebEnginePage::SelectEndOfLine);
            }
            else if (event->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier)) {
                triggerPageAction(QWebEnginePage::SelectEndOfDocument);
            }
            event->accept();
            return;
        }
        break;

    case Qt::Key_Insert:
        if (event->modifiers() == Qt::ControlModifier) {
            triggerPageAction(QWebEnginePage::Copy);
            event->accept();
            return;
        }
        if (event->modifiers() == Qt::ShiftModifier) {
            triggerPageAction(QWebEnginePage::Paste);
            event->accept();
            return;
        }
        break;

    default:
        break;
    }
#endif

    QWebEngineView::keyPressEvent(event);
}

void WebView::keyReleaseEvent(QKeyEvent* event)
{
    if (mApp->plugins()->processKeyRelease(Qz::ON_WebView, this, event)) {
        return;
    }

    QWebEngineView::keyReleaseEvent(event);
}

void WebView::resizeEvent(QResizeEvent* event)
{
    QWebEngineView::resizeEvent(event);
    emit viewportResized(size());
}

void WebView::loadRequest(const LoadRequest &req)
{
    if (req.operation() == LoadRequest::GetOperation)
        load(req.url());
    else
        m_page->runJavaScript(Scripts::sendPostData(req.url(), req.data()));
}

bool WebView::eventFilter(QObject* obj, QEvent* event)
{
    if (s_forceContextMenuOnMouseRelease && obj == this && event->type() == QEvent::ContextMenu) {
        QContextMenuEvent* ev = static_cast<QContextMenuEvent*>(event);
        if (ev->reason() == QContextMenuEvent::Mouse && ev->spontaneous()) {
            ev->accept();
            return true;
        }
    }

    return QWebEngineView::eventFilter(obj, event);
}
