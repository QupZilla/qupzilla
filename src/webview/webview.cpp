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

WebView::WebView(QWidget* parent)
    : QWebView(parent)
    , m_currentZoom(100)
    , m_isLoading(false)
    , m_progress(0)
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

QMenu *WebView::createMediaContextMenu(const QWebHitTestResult &hitTest)
{
    QMenu* menu = new QMenu(this);
    m_mediaElement = hitTest.element();

    bool paused = m_mediaElement.evaluateJavaScript("this.paused").toBool();
    bool muted = m_mediaElement.evaluateJavaScript("this.muted").toBool();
    QUrl videoUrl = m_mediaElement.evaluateJavaScript("this.currentSrc").toUrl();

    menu->addAction(paused ? tr("&Play") : tr("&Pause"), this, SLOT(pauseMedia()))->setIcon(QIcon::fromTheme(paused ? "media-playback-start" : "media-playback-pause"));
    menu->addAction(muted ? tr("Un&mute") : tr("&Mute"), this, SLOT(muteMedia()))->setIcon(QIcon::fromTheme(muted ? "audio-volume-muted" : "audio-volume-high"));
    menu->addSeparator();
    menu->addAction(QIcon::fromTheme("edit-copy"), tr("&Copy Media Address"), this, SLOT(copyLinkToClipboard()))->setData(videoUrl);
    menu->addAction(QIcon::fromTheme("mail-message-new"), tr("&Send Media Address"), this, SLOT(sendLinkByMail()))->setData(videoUrl);
    menu->addAction(QIcon::fromTheme("download"), tr("&Download Media To Disk"), this, SLOT(downloadLinkToDisk()))->setData(videoUrl);

    return menu;
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

void WebView::controlsMedia()
{
    bool controls= m_mediaElement.evaluateJavaScript("this.controls").toBool();

    if (controls) {
        m_mediaElement.evaluateJavaScript("this.controls = false");
    }
    else {
        m_mediaElement.evaluateJavaScript("this.controls = true");
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

    default:
        QWebView::mousePressEvent(event);
        break;
    }
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

