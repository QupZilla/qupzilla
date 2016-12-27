/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2016  David Rosca <nowrep@gmail.com>
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
#include "webhittestresult.h"
#include "webscrollbarmanager.h"

#include <iostream>

#include <QDir>
#include <QTimer>
#include <QDesktopServices>
#include <QWebEngineHistory>
#include <QClipboard>
#include <QHostInfo>
#include <QMimeData>
#include <QWebEngineContextMenuData>
#include <QStackedLayout>
#include <QScrollBar>

bool WebView::s_forceContextMenuOnMouseRelease = false;

WebView::WebView(QWidget* parent)
    : QWebEngineView(parent)
    , m_progress(100)
    , m_page(0)
    , m_firstLoad(false)
{
    connect(this, &QWebEngineView::loadStarted, this, &WebView::slotLoadStarted);
    connect(this, &QWebEngineView::loadProgress, this, &WebView::slotLoadProgress);
    connect(this, &QWebEngineView::loadFinished, this, &WebView::slotLoadFinished);
    connect(this, &QWebEngineView::iconChanged, this, &WebView::slotIconChanged);
    connect(this, &QWebEngineView::urlChanged, this, &WebView::slotUrlChanged);

    m_currentZoomLevel = zoomLevels().indexOf(100);

    setAcceptDrops(true);
    installEventFilter(this);
    if (parentWidget()) {
        parentWidget()->installEventFilter(this);
    }

    WebInspector::registerView(this);

    // Hack to find widget that receives input events
    QStackedLayout *l = qobject_cast<QStackedLayout*>(layout());
    connect(l, &QStackedLayout::currentChanged, this, [this]() {
        QTimer::singleShot(0, this, [this]() {
            m_rwhvqt = focusProxy();
            if (!m_rwhvqt) {
                qCritical() << "Focus proxy is null!";
                return;
            }
            m_rwhvqt->installEventFilter(this);
        });
    });
}

WebView::~WebView()
{
    WebInspector::unregisterView(this);
    WebScrollBarManager::instance()->removeWebView(this);
}

QIcon WebView::icon() const
{
    if (!QWebEngineView::icon().isNull()) {
        return QWebEngineView::icon();
    }

    if (url().scheme() == QLatin1String("ftp")) {
        return IconProvider::standardIcon(QStyle::SP_ComputerIcon);
    }

    if (url().scheme() == QLatin1String("file")) {
        return IconProvider::standardIcon(QStyle::SP_DriveHDIcon);
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

    m_page = page;
    m_page->setParent(this);
    QWebEngineView::setPage(m_page);

    connect(m_page, SIGNAL(privacyChanged(bool)), this, SIGNAL(privacyChanged(bool)));

    // Set default zoom level
    zoomReset();

    // Actions needs to be initialized for every QWebEnginePage change
    initializeActions();

    // Scrollbars must be added only after QWebEnginePage is set
    WebScrollBarManager::instance()->addWebView(this);

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

    if (reqUrl.isEmpty())
        return;

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

    if (isUrlValid(reqUrl)) {
        loadRequest(request);
        return;
    }

    // Make sure to correctly load hosts like localhost (eg. without the dot)
    if (!reqUrl.isEmpty() &&
        reqUrl.scheme().isEmpty() &&
        !QzTools::containsSpace(reqUrl.path()) && // See #1622
        !reqUrl.path().contains(QL1C('.'))
       ) {
        QUrl u(QSL("http://") + reqUrl.path());
        if (u.isValid()) {
            // This is blocking...
            QHostInfo info = QHostInfo::fromName(u.path());
            if (info.error() == QHostInfo::NoError) {
                LoadRequest req = request;
                req.setUrl(u);
                loadRequest(req);
                return;
            }
        }
    }

    if (qzSettings->searchFromAddressBar) {
        const LoadRequest searchRequest = mApp->searchEnginesManager()->searchResult(request.urlString());
        loadRequest(searchRequest);
    }
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

QPointF WebView::mapToViewport(const QPointF &pos) const
{
    return page()->mapToViewport(pos);
}

QRect WebView::scrollBarGeometry(Qt::Orientation orientation) const
{
    QScrollBar *s = WebScrollBarManager::instance()->scrollBar(orientation, const_cast<WebView*>(this));
    return s && s->isVisible() ? s->geometry() : QRect();
}

void WebView::restoreHistory(const QByteArray &data)
{
    QDataStream stream(data);
    stream >> *history();

    // Workaround clearing QWebChannel after restoring history
    page()->setupWebChannel();
}

QWidget *WebView::inputWidget() const
{
    return m_rwhvqt ? m_rwhvqt : const_cast<WebView*>(this);
}

// static
bool WebView::isUrlValid(const QUrl &url)
{
    // Valid url must have scheme and actually contains something (therefore scheme:// is invalid)
    return url.isValid() && !url.scheme().isEmpty() && (!url.host().isEmpty() || !url.path().isEmpty() || url.hasQuery());
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

void WebView::slotLoadFinished(bool ok)
{
    m_progress = 100;

    if (ok)
        mApp->history()->addHistoryEntry(this);
}

void WebView::slotIconChanged()
{
    IconProvider::instance()->saveIcon(this);
}

void WebView::slotUrlChanged(const QUrl &url)
{
    Q_UNUSED(url)

    // Don't save blank page / speed dial in tab history
    if (!history()->canGoForward()  && history()->backItems(1).size() == 1) {
        const QString s = LocationBar::convertUrlToText(history()->backItem().url());
        if (s.isEmpty())
            history()->clear();
    }
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
    triggerPageAction(QWebEnginePage::SavePage);
}

void WebView::copyImageToClipboard()
{
    triggerPageAction(QWebEnginePage::CopyImageToClipboard);
}

void WebView::downloadLinkToDisk()
{
    triggerPageAction(QWebEnginePage::DownloadLinkToDisk);
}

void WebView::downloadImageToDisk()
{
    triggerPageAction(QWebEnginePage::DownloadImageToDisk);
}

void WebView::downloadMediaToDisk()
{
    triggerPageAction(QWebEnginePage::DownloadMediaToDisk);
}

void WebView::openUrlInNewTab(const QUrl &url, Qz::NewTabPositionFlags position)
{
    loadInNewTab(url, position);
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

#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
    triggerPageAction(QWebEnginePage::ViewSource);
#else
    QUrl u;
    u.setScheme(QSL("view-source"));
    u.setPath(url().toString());
    openUrlInNewTab(u, Qz::NT_SelectedTab);
#endif
}

void WebView::showSiteInfo()
{
    SiteInfo* s = new SiteInfo(this);
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

    const LoadRequest req = mApp->searchEnginesManager()->searchResult(engine, selectedText());
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

    const LoadRequest req = mApp->searchEnginesManager()->searchResult(engine, selectedText());
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

void WebView::createContextMenu(QMenu *menu, WebHitTestResult &hitTest)
{
    // cppcheck-suppress variableScope
    int spellCheckActionCount = 0;

    const QWebEngineContextMenuData &contextMenuData = page()->contextMenuData();
    hitTest.updateWithContextMenuData(contextMenuData);

#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
    if (!contextMenuData.misspelledWord().isEmpty()) {
        QFont boldFont = menu->font();
        boldFont.setBold(true);

        for (const QString &suggestion : contextMenuData.spellCheckerSuggestions()) {
            QAction *action = menu->addAction(suggestion);
            action->setFont(boldFont);

            connect(action, &QAction::triggered, this, [=]() {
                page()->replaceMisspelledWord(suggestion);
            });
        }

        if (menu->actions().isEmpty()) {
            menu->addAction(tr("No suggestions"))->setEnabled(false);
        }

        menu->addSeparator();
        spellCheckActionCount = menu->actions().count();
    }
#endif

    if (!hitTest.linkUrl().isEmpty() && hitTest.linkUrl().scheme() != QL1S("javascript")) {
        createLinkContextMenu(menu, hitTest);
    }

    if (!hitTest.imageUrl().isEmpty()) {
        createImageContextMenu(menu, hitTest);
    }

    if (!hitTest.mediaUrl().isEmpty()) {
        createMediaContextMenu(menu, hitTest);
    }

    if (hitTest.isContentEditable()) {
        // This only checks if the menu is empty (only spellchecker actions added)
        if (menu->actions().count() == spellCheckActionCount) {
            menu->addAction(pageAction(QWebEnginePage::Undo));
            menu->addAction(pageAction(QWebEnginePage::Redo));
            menu->addSeparator();
            menu->addAction(pageAction(QWebEnginePage::Cut));
            menu->addAction(pageAction(QWebEnginePage::Copy));
            menu->addAction(pageAction(QWebEnginePage::Paste));
        }

        if (hitTest.tagName() == QL1S("input")) {
            QAction *act = menu->addAction(QString());
            act->setVisible(false);
            checkForForm(act, hitTest.pos());
        }
    }

    if (!selectedText().isEmpty()) {
        createSelectedTextContextMenu(menu, hitTest);
    }

    if (menu->isEmpty()) {
        createPageContextMenu(menu);
    }

    menu->addSeparator();
    mApp->plugins()->populateWebViewMenu(menu, this, hitTest);
}

void WebView::createPageContextMenu(QMenu* menu)
{
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

    menu->addAction(pageAction(QWebEnginePage::Reload));
    menu->addAction(pageAction(QWebEnginePage::Stop));
    menu->addSeparator();
    menu->addAction(QIcon::fromTheme("bookmark-new"), tr("Book&mark page"), this, SLOT(bookmarkLink()));
    menu->addAction(QIcon::fromTheme("document-save"), tr("&Save page as..."), this, SLOT(savePageAs()));
    menu->addAction(QIcon::fromTheme("edit-copy"), tr("&Copy page link"), this, SLOT(copyLinkToClipboard()))->setData(url());
    menu->addAction(QIcon::fromTheme("mail-message-new"), tr("Send page link..."), this, SLOT(sendPageByMail()));
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

    if (SiteInfo::canShowSiteInfo(url()))
        menu->addAction(QIcon::fromTheme("dialog-information"), tr("Show info ab&out site"), this, SLOT(showSiteInfo()));
}

void WebView::createLinkContextMenu(QMenu* menu, const WebHitTestResult &hitTest)
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

    menu->addAction(QIcon::fromTheme("document-save"), tr("&Save link as..."), this, SLOT(downloadLinkToDisk()));
    menu->addAction(QIcon::fromTheme("mail-message-new"), tr("Send link..."), this, SLOT(sendLinkByMail()))->setData(hitTest.linkUrl());
    menu->addAction(QIcon::fromTheme("edit-copy"), tr("&Copy link address"), this, SLOT(copyLinkToClipboard()))->setData(hitTest.linkUrl());
    menu->addSeparator();

    if (!selectedText().isEmpty()) {
        pageAction(QWebEnginePage::Copy)->setIcon(QIcon::fromTheme("edit-copy"));
        menu->addAction(pageAction(QWebEnginePage::Copy));
    }
}

void WebView::createImageContextMenu(QMenu* menu, const WebHitTestResult &hitTest)
{
    menu->addSeparator();
    Action* act = new Action(tr("Show i&mage"));
    act->setData(hitTest.imageUrl());
    connect(act, SIGNAL(triggered()), this, SLOT(openActionUrl()));
    connect(act, SIGNAL(ctrlTriggered()), this, SLOT(userDefinedOpenUrlInNewTab()));
    menu->addAction(act);
    menu->addAction(tr("Copy image"), this, SLOT(copyImageToClipboard()));
    menu->addAction(QIcon::fromTheme("edit-copy"), tr("Copy image ad&dress"), this, SLOT(copyLinkToClipboard()))->setData(hitTest.imageUrl());
    menu->addSeparator();
    menu->addAction(QIcon::fromTheme("document-save"), tr("&Save image as..."), this, SLOT(downloadImageToDisk()));
    menu->addAction(QIcon::fromTheme("mail-message-new"), tr("Send image..."), this, SLOT(sendLinkByMail()))->setData(hitTest.imageUrl());
    menu->addSeparator();

    if (!selectedText().isEmpty()) {
        pageAction(QWebEnginePage::Copy)->setIcon(QIcon::fromTheme("edit-copy"));
        menu->addAction(pageAction(QWebEnginePage::Copy));
    }
}

void WebView::createSelectedTextContextMenu(QMenu* menu, const WebHitTestResult &hitTest)
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

void WebView::createMediaContextMenu(QMenu *menu, const WebHitTestResult &hitTest)
{
    bool paused = hitTest.mediaPaused();
    bool muted = hitTest.mediaMuted();

    menu->addSeparator();
    menu->addAction(paused ? tr("&Play") : tr("&Pause"), this, SLOT(toggleMediaPause()))->setIcon(QIcon::fromTheme(paused ? "media-playback-start" : "media-playback-pause"));
    menu->addAction(muted ? tr("Un&mute") : tr("&Mute"), this, SLOT(toggleMediaMute()))->setIcon(QIcon::fromTheme(muted ? "audio-volume-muted" : "audio-volume-high"));
    menu->addSeparator();
    menu->addAction(QIcon::fromTheme("edit-copy"), tr("&Copy Media Address"), this, SLOT(copyLinkToClipboard()))->setData(hitTest.mediaUrl());
    menu->addAction(QIcon::fromTheme("mail-message-new"), tr("&Send Media Address"), this, SLOT(sendLinkByMail()))->setData(hitTest.mediaUrl());
    menu->addAction(QIcon::fromTheme("document-save"), tr("Save Media To &Disk"), this, SLOT(downloadMediaToDisk()));
}

void WebView::checkForForm(QAction *action, const QPoint &pos)
{
    m_clickedPos = mapToViewport(pos);
    QPointer<QAction> act = action;

    page()->runJavaScript(Scripts::getFormData(m_clickedPos), WebPage::SafeJsWorld, [this, act](const QVariant &res) {
        const QVariantMap &map = res.toMap();
        if (!act || map.isEmpty())
            return;

        const QUrl url = map.value(QSL("action")).toUrl();
        const QString method = map.value(QSL("method")).toString();

        if (!url.isEmpty() && (method == QL1S("get") || method == QL1S("post"))) {
            act->setVisible(true);
            act->setIcon(QIcon(QSL(":icons/menu/search-icon.png")));
            act->setText(tr("Create Search Engine"));
            connect(act.data(), &QAction::triggered, this, &WebView::createSearchEngine);
        }
    });
}

void WebView::createSearchEngine()
{
    page()->runJavaScript(Scripts::getFormData(m_clickedPos), WebPage::SafeJsWorld, [this](const QVariant &res) {
        mApp->searchEnginesManager()->addEngineFromForm(res.toMap(), this);
    });
}

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

void WebView::toggleMediaPause()
{
    triggerPageAction(QWebEnginePage::ToggleMediaPlayPause);
}

void WebView::toggleMediaMute()
{
    triggerPageAction(QWebEnginePage::ToggleMediaMute);
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

void WebView::_wheelEvent(QWheelEvent *event)
{
    if (mApp->plugins()->processWheelEvent(Qz::ON_WebView, this, event)) {
        event->accept();
        return;
    }

    if (event->modifiers() & Qt::ControlModifier) {
        event->delta() > 0 ? zoomIn() : zoomOut();
        event->accept();
    }
}

void WebView::_mousePressEvent(QMouseEvent *event)
{
    m_clickedUrl = QUrl();
    m_clickedPos = QPointF();

    if (mApp->plugins()->processMousePress(Qz::ON_WebView, this, event)) {
        event->accept();
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

    case Qt::MiddleButton:
        m_clickedUrl = page()->hitTestContent(event->pos()).linkUrl();
        if (!m_clickedUrl.isEmpty())
            event->accept();
        break;

    case Qt::LeftButton:
        m_clickedUrl = page()->hitTestContent(event->pos()).linkUrl();
        break;

    default:
        break;
    }
}

void WebView::_mouseReleaseEvent(QMouseEvent *event)
{
    if (mApp->plugins()->processMouseRelease(Qz::ON_WebView, this, event)) {
        event->accept();
        return;
    }

    switch (event->button()) {
    case Qt::MiddleButton:
        if (!m_clickedUrl.isEmpty()) {
            const QUrl link = page()->hitTestContent(event->pos()).linkUrl();
            if (m_clickedUrl == link && isUrlValid(link)) {
                userDefinedOpenUrlInNewTab(link, event->modifiers() & Qt::ShiftModifier);
                event->accept();
            }
        }
        break;

    case Qt::LeftButton:
        if (!m_clickedUrl.isEmpty()) {
            const QUrl link = page()->hitTestContent(event->pos()).linkUrl();
            if (m_clickedUrl == link && isUrlValid(link)) {
                if (event->modifiers() & Qt::ControlModifier) {
                    userDefinedOpenUrlInNewTab(link, event->modifiers() & Qt::ShiftModifier);
                    event->accept();
                }
            }
        }
        break;

    case Qt::RightButton:
        if (s_forceContextMenuOnMouseRelease) {
            QContextMenuEvent ev(QContextMenuEvent::Mouse, event->pos(), event->globalPos(), event->modifiers());
            _contextMenuEvent(&ev);
            event->accept();
        }
        break;

    default:
        break;
    }
}

void WebView::_mouseMoveEvent(QMouseEvent *event)
{
    if (mApp->plugins()->processMouseMove(Qz::ON_WebView, this, event)) {
        event->accept();
    }
}

void WebView::_keyPressEvent(QKeyEvent *event)
{
    if (mApp->plugins()->processKeyPress(Qz::ON_WebView, this, event)) {
        event->accept();
        return;
    }

    switch (event->key()) {
    case Qt::Key_ZoomIn:
        zoomIn();
        event->accept();
        break;

    case Qt::Key_ZoomOut:
        zoomOut();
        event->accept();
        break;

    case Qt::Key_Plus:
        if (event->modifiers() & Qt::ControlModifier) {
            zoomIn();
            event->accept();
        }
        break;

    case Qt::Key_Minus:
        if (event->modifiers() & Qt::ControlModifier) {
            zoomOut();
            event->accept();
        }
        break;

    case Qt::Key_0:
        if (event->modifiers() & Qt::ControlModifier) {
            zoomReset();
            event->accept();
        }
        break;

    case Qt::Key_M:
        if (event->modifiers() & Qt::ControlModifier) {
            page()->setAudioMuted(!page()->isAudioMuted());
            event->accept();
        }
        break;

    default:
        break;
    }
}

void WebView::_keyReleaseEvent(QKeyEvent *event)
{
    if (mApp->plugins()->processKeyRelease(Qz::ON_WebView, this, event)) {
        event->accept();
    }

    switch (event->key()) {
    case Qt::Key_Escape:
        if (isFullScreen()) {
            triggerPageAction(QWebEnginePage::ExitFullScreen);
            event->accept();
        }
        break;

    default:
        break;
    }
}

void WebView::_contextMenuEvent(QContextMenuEvent *event)
{
    Q_UNUSED(event)
}

void WebView::resizeEvent(QResizeEvent *event)
{
    QWebEngineView::resizeEvent(event);
    emit viewportResized(size());
}

void WebView::contextMenuEvent(QContextMenuEvent *event)
{
    // Context menu is created in mouseReleaseEvent
    if (s_forceContextMenuOnMouseRelease)
        return;

    const QPoint pos = event->pos();
    const QContextMenuEvent::Reason reason = event->reason();

    QTimer::singleShot(0, this, [this, pos, reason]() {
        QContextMenuEvent ev(reason, pos);
        _contextMenuEvent(&ev);
    });
}

void WebView::loadRequest(const LoadRequest &req)
{
    if (req.operation() == LoadRequest::GetOperation)
        load(req.url());
    else
        page()->runJavaScript(Scripts::sendPostData(req.url(), req.data()), WebPage::SafeJsWorld);
}

bool WebView::eventFilter(QObject *obj, QEvent *event)
{
    // Keyboard events are sent to parent widget
    if (obj == this && event->type() == QEvent::ParentChange && parentWidget()) {
        parentWidget()->installEventFilter(this);
    }

    // Forward events to WebView
#define HANDLE_EVENT(f, t) \
    { \
    bool wasAccepted = event->isAccepted(); \
    event->setAccepted(false); \
    f(static_cast<t*>(event)); \
    bool ret = event->isAccepted(); \
    event->setAccepted(wasAccepted); \
    return ret; \
    }

    if (obj == m_rwhvqt) {
        switch (event->type()) {
        case QEvent::MouseButtonPress:
            HANDLE_EVENT(_mousePressEvent, QMouseEvent);

        case QEvent::MouseButtonRelease:
            HANDLE_EVENT(_mouseReleaseEvent, QMouseEvent);

        case QEvent::MouseMove:
            HANDLE_EVENT(_mouseMoveEvent, QMouseEvent);

        case QEvent::Wheel:
            HANDLE_EVENT(_wheelEvent, QWheelEvent);

        default:
            break;
        }
    }

    if (obj == parentWidget()) {
        switch (event->type()) {
        case QEvent::KeyPress:
            HANDLE_EVENT(_keyPressEvent, QKeyEvent);

        case QEvent::KeyRelease:
            HANDLE_EVENT(_keyReleaseEvent, QKeyEvent);

        default:
            break;
        }
    }
#undef HANDLE_EVENT

    // Block already handled events
    if (obj == this) {
        switch (event->type()) {
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseMove:
        case QEvent::Wheel:
            return true;

        default:
            break;
        }
    }

    const bool res = QWebEngineView::eventFilter(obj, event);

    if (obj == m_rwhvqt) {
        switch (event->type()) {
        case QEvent::FocusIn:
        case QEvent::FocusOut:
            emit focusChanged(hasFocus());
            break;

        default:
            break;
        }
    }

    return res;
}
