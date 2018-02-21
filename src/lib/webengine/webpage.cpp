/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018 David Rosca <nowrep@gmail.com>
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
#include "webpage.h"
#include "tabbedwebview.h"
#include "browserwindow.h"
#include "pluginproxy.h"
#include "downloadmanager.h"
#include "mainapplication.h"
#include "checkboxdialog.h"
#include "widget.h"
#include "qztools.h"
#include "speeddial.h"
#include "autofill.h"
#include "popupwebview.h"
#include "popupwindow.h"
#include "iconprovider.h"
#include "qzsettings.h"
#include "useragentmanager.h"
#include "delayedfilewatcher.h"
#include "searchenginesmanager.h"
#include "html5permissions/html5permissionsmanager.h"
#include "javascript/externaljsobject.h"
#include "tabwidget.h"
#include "networkmanager.h"
#include "webhittestresult.h"
#include "ui_jsconfirm.h"
#include "ui_jsalert.h"
#include "ui_jsprompt.h"
#include "passwordmanager.h"

#include <iostream>

#include <QDir>
#include <QMouseEvent>
#include <QWebChannel>
#include <QWebEngineHistory>
#include <QWebEngineSettings>
#include <QTimer>
#include <QDesktopServices>
#include <QMessageBox>
#include <QFileDialog>
#include <QAuthenticator>
#include <QPushButton>
#include <QUrlQuery>

QString WebPage::s_lastUploadLocation = QDir::homePath();
QUrl WebPage::s_lastUnsupportedUrl;
QTime WebPage::s_lastUnsupportedUrlTime;

static const bool kEnableJsOutput = qEnvironmentVariableIsSet("QUPZILLA_ENABLE_JS_OUTPUT");
static const bool kEnableJsNonBlockDialogs = qEnvironmentVariableIsSet("QUPZILLA_ENABLE_JS_NONBLOCK_DIALOGS");

WebPage::WebPage(QObject* parent)
    : QWebEnginePage(mApp->webProfile(), parent)
    , m_fileWatcher(0)
    , m_runningLoop(0)
    , m_loadProgress(100)
    , m_blockAlerts(false)
    , m_secureStatus(false)
{
    connect(this, &QWebEnginePage::loadProgress, this, &WebPage::progress);
    connect(this, &QWebEnginePage::loadFinished, this, &WebPage::finished);
    connect(this, &QWebEnginePage::urlChanged, this, &WebPage::urlChanged);
    connect(this, &QWebEnginePage::featurePermissionRequested, this, &WebPage::featurePermissionRequested);
    connect(this, &QWebEnginePage::windowCloseRequested, this, &WebPage::windowCloseRequested);
    connect(this, &QWebEnginePage::fullScreenRequested, this, &WebPage::fullScreenRequested);
    connect(this, &QWebEnginePage::renderProcessTerminated, this, &WebPage::renderProcessTerminated);

    connect(this, &QWebEnginePage::authenticationRequired, this, [this](const QUrl &url, QAuthenticator *auth) {
        mApp->networkManager()->authentication(url, auth, view());
    });

    connect(this, &QWebEnginePage::proxyAuthenticationRequired, this, [this](const QUrl &, QAuthenticator *auth, const QString &proxyHost) {
        mApp->networkManager()->proxyAuthentication(proxyHost, auth, view());
    });

    // Workaround QWebEnginePage not scrolling to anchors when opened in background tab
    m_contentsResizedConnection = connect(this, &QWebEnginePage::contentsSizeChanged, this, [this]() {
        const QString fragment = url().fragment();
        if (!fragment.isEmpty()) {
            const QString src = QSL("var els = document.querySelectorAll(\"[name='%1']\"); if (els.length) els[0].scrollIntoView();");
            runJavaScript(src.arg(fragment));
        }
        disconnect(m_contentsResizedConnection);
    });

    // Workaround for broken load started/finished signals in QtWebEngine 5.10
    if (qstrncmp(qVersion(), "5.10.", 5) == 0) {
        connect(this, &QWebEnginePage::loadProgress, this, [this](int progress) {
            if (progress == 100) {
                emit loadFinished(true);
            }
        });
    }

    // Workaround for changing webchannel world inside acceptNavigationRequest not working
    m_setupChannelTimer = new QTimer(this);
    m_setupChannelTimer->setSingleShot(true);
    m_setupChannelTimer->setInterval(100);
    connect(m_setupChannelTimer, &QTimer::timeout, this, [this]() {
        setupWebChannelForUrl(m_channelUrl);
    });
}

WebPage::~WebPage()
{
    if (m_runningLoop) {
        m_runningLoop->exit(1);
        m_runningLoop = 0;
    }
}

WebView *WebPage::view() const
{
    return static_cast<WebView*>(QWebEnginePage::view());
}

bool WebPage::execPrintPage(QPrinter *printer, int timeout)
{
    QPointer<QEventLoop> loop = new QEventLoop;
    bool result = false;
    QTimer::singleShot(timeout, loop.data(), &QEventLoop::quit);

    print(printer, [loop, &result](bool res) {
        if (loop && loop->isRunning()) {
            result = res;
            loop->quit();
        }
    });

    loop->exec();
    delete loop;

    return result;
}

QVariant WebPage::execJavaScript(const QString &scriptSource, quint32 worldId, int timeout)
{
    QPointer<QEventLoop> loop = new QEventLoop;
    QVariant result;
    QTimer::singleShot(timeout, loop.data(), &QEventLoop::quit);

    runJavaScript(scriptSource, worldId, [loop, &result](const QVariant &res) {
        if (loop && loop->isRunning()) {
            result = res;
            loop->quit();
        }
    });

    loop->exec(QEventLoop::ExcludeUserInputEvents);
    delete loop;

    return result;
}

QPointF WebPage::mapToViewport(const QPointF &pos) const
{
    return QPointF(pos.x() / zoomFactor(), pos.y() / zoomFactor());
}

WebHitTestResult WebPage::hitTestContent(const QPoint &pos) const
{
    return WebHitTestResult(this, pos);
}

void WebPage::scroll(int x, int y)
{
    runJavaScript(QSL("window.scrollTo(window.scrollX + %1, window.scrollY + %2)").arg(x).arg(y), SafeJsWorld);
}

void WebPage::setScrollPosition(const QPointF &pos)
{
    const QPointF v = mapToViewport(pos.toPoint());
    runJavaScript(QSL("window.scrollTo(%1, %2)").arg(v.x()).arg(v.y()), SafeJsWorld);
}

bool WebPage::isRunningLoop()
{
    return m_runningLoop;
}

bool WebPage::isLoading() const
{
    return m_loadProgress < 100;
}

void WebPage::urlChanged(const QUrl &url)
{
    Q_UNUSED(url)

    if (isLoading()) {
        m_blockAlerts = false;
    }
}

void WebPage::progress(int prog)
{
    m_loadProgress = prog;

    bool secStatus = url().scheme() == QL1S("https");

    if (secStatus != m_secureStatus) {
        m_secureStatus = secStatus;
        emit privacyChanged(secStatus);
    }
}

void WebPage::finished()
{
    progress(100);

    // File scheme watcher
    if (url().scheme() == QLatin1String("file")) {
        QFileInfo info(url().toLocalFile());
        if (info.isFile()) {
            if (!m_fileWatcher) {
                m_fileWatcher = new DelayedFileWatcher(this);
                connect(m_fileWatcher, SIGNAL(delayedFileChanged(QString)), this, SLOT(watchedFileChanged(QString)));
            }

            const QString filePath = url().toLocalFile();

            if (QFile::exists(filePath) && !m_fileWatcher->files().contains(filePath)) {
                m_fileWatcher->addPath(filePath);
            }
        }
    }
    else if (m_fileWatcher && !m_fileWatcher->files().isEmpty()) {
        m_fileWatcher->removePaths(m_fileWatcher->files());
    }

    // AutoFill
    m_autoFillUsernames = mApp->autoFill()->completePage(this, url());
}

void WebPage::watchedFileChanged(const QString &file)
{
    if (url().toLocalFile() == file) {
        triggerAction(QWebEnginePage::Reload);
    }
}

void WebPage::handleUnknownProtocol(const QUrl &url)
{
    const QString protocol = url.scheme();

    if (protocol == QLatin1String("mailto")) {
        desktopServicesOpen(url);
        return;
    }

    if (qzSettings->blockedProtocols.contains(protocol)) {
        qDebug() << "WebPage::handleUnknownProtocol Protocol" << protocol << "is blocked!";
        return;
    }

    if (qzSettings->autoOpenProtocols.contains(protocol)) {
        desktopServicesOpen(url);
        return;
    }

    CheckBoxDialog dialog(QMessageBox::Yes | QMessageBox::No, view());
    dialog.setDefaultButton(QMessageBox::Yes);

    const QString wrappedUrl = QzTools::alignTextToWidth(url.toString(), "<br/>", dialog.fontMetrics(), 450);
    const QString text = tr("QupZilla cannot handle <b>%1:</b> links. The requested link "
                            "is <ul><li>%2</li></ul>Do you want QupZilla to try "
                            "open this link in system application?").arg(protocol, wrappedUrl);

    dialog.setText(text);
    dialog.setCheckBoxText(tr("Remember my choice for this protocol"));
    dialog.setWindowTitle(tr("External Protocol Request"));
    dialog.setIcon(QMessageBox::Question);

    switch (dialog.exec()) {
    case QMessageBox::Yes:
        if (dialog.isChecked()) {
            qzSettings->autoOpenProtocols.append(protocol);
            qzSettings->saveSettings();
        }


        QDesktopServices::openUrl(url);
        break;

    case QMessageBox::No:
        if (dialog.isChecked()) {
            qzSettings->blockedProtocols.append(protocol);
            qzSettings->saveSettings();
        }

        break;

    default:
        break;
    }
}

void WebPage::desktopServicesOpen(const QUrl &url)
{
    // Open same url only once in 2 secs
    const int sameUrlTimeout = 2 * 1000;

    if (s_lastUnsupportedUrl != url || s_lastUnsupportedUrlTime.isNull() || s_lastUnsupportedUrlTime.elapsed() > sameUrlTimeout) {
        s_lastUnsupportedUrl = url;
        s_lastUnsupportedUrlTime.restart();
        QDesktopServices::openUrl(url);
    }
    else {
        qWarning() << "WebPage::desktopServicesOpen Url" << url << "has already been opened!\n"
                   "Ignoring it to prevent infinite loop!";
    }
}

void WebPage::windowCloseRequested()
{
    if (!view())
        return;
    view()->closeView();
}

void WebPage::fullScreenRequested(QWebEngineFullScreenRequest fullScreenRequest)
{
    view()->requestFullScreen(fullScreenRequest.toggleOn());

    const bool accepted = fullScreenRequest.toggleOn() == view()->isFullScreen();

    if (accepted)
        fullScreenRequest.accept();
    else
        fullScreenRequest.reject();
}

void WebPage::featurePermissionRequested(const QUrl &origin, const QWebEnginePage::Feature &feature)
{
    if (feature == MouseLock && view()->isFullScreen())
        setFeaturePermission(origin, feature, PermissionGrantedByUser);
    else
        mApp->html5PermissionsManager()->requestPermissions(this, origin, feature);
}

void WebPage::renderProcessTerminated(QWebEnginePage::RenderProcessTerminationStatus terminationStatus, int exitCode)
{
    Q_UNUSED(exitCode)

    if (terminationStatus == NormalTerminationStatus)
        return;

    QTimer::singleShot(0, this, [this]() {
        QString page = QzTools::readAllFileContents(":html/tabcrash.html");
        page.replace(QL1S("%IMAGE%"), QzTools::pixmapToDataUrl(IconProvider::standardIcon(QStyle::SP_MessageBoxWarning).pixmap(45)).toString());
        page.replace(QL1S("%TITLE%"), tr("Failed loading page"));
        page.replace(QL1S("%HEADING%"), tr("Failed loading page"));
        page.replace(QL1S("%LI-1%"), tr("Something went wrong while loading this page."));
        page.replace(QL1S("%LI-2%"), tr("Try reloading the page or closing some tabs to make more memory available."));
        page.replace(QL1S("%RELOAD-PAGE%"), tr("Reload page"));
        page = QzTools::applyDirectionToPage(page);
        setHtml(page.toUtf8(), url());
    });
}

void WebPage::setupWebChannelForUrl(const QUrl &url)
{
    QWebChannel *channel = webChannel();
    if (!channel) {
        channel = new QWebChannel(this);
        ExternalJsObject::setupWebChannel(channel, this);
    }
    int worldId = -1;
    if (url.scheme() == QL1S("qupzilla") || url.scheme() == QL1S("extension")) {
        worldId = UnsafeJsWorld;
    } else {
        worldId = SafeJsWorld;
    }
    if (worldId != m_channelWorldId) {
        m_channelWorldId = worldId;
        setWebChannel(channel, m_channelWorldId);
    }
}

bool WebPage::acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame)
{
    if (mApp->isClosing()) {
        return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
    }

    if (!mApp->plugins()->acceptNavigationRequest(this, url, type, isMainFrame))
        return false;

    if (url.scheme() == QL1S("qupzilla")) {
        if (url.path() == QL1S("AddSearchProvider")) {
            QUrlQuery query(url);
            mApp->searchEnginesManager()->addEngine(QUrl(query.queryItemValue(QSL("url"))));
            return false;
        } else if (url.path() == QL1S("PrintPage")) {
            emit printRequested();
            return false;
        }
    }

    const bool result = QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);

    if (result) {
        if (isMainFrame) {
            const bool isWeb = url.scheme() == QL1S("http") || url.scheme() == QL1S("https") || url.scheme() == QL1S("file");
            const bool globalJsEnabled = mApp->webSettings()->testAttribute(QWebEngineSettings::JavascriptEnabled);
            settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, isWeb ? globalJsEnabled : true);
            m_channelUrl = url;
            m_setupChannelTimer->start();
        }
        emit navigationRequestAccepted(url, type, isMainFrame);
    }

    return result;
}

bool WebPage::certificateError(const QWebEngineCertificateError &error)
{
    return mApp->networkManager()->certificateError(error, view());
}

QStringList WebPage::chooseFiles(QWebEnginePage::FileSelectionMode mode, const QStringList &oldFiles, const QStringList &acceptedMimeTypes)
{
    Q_UNUSED(acceptedMimeTypes);

    QStringList files;
    QString suggestedFileName = s_lastUploadLocation;
    if (!oldFiles.isEmpty())
        suggestedFileName = oldFiles.at(0);

    switch (mode) {
    case FileSelectOpen:
        files = QStringList(QzTools::getOpenFileName("WebPage-ChooseFile", view(), tr("Choose file..."), suggestedFileName));
        break;

    case FileSelectOpenMultiple:
        files = QzTools::getOpenFileNames("WebPage-ChooseFile", view(), tr("Choose files..."), suggestedFileName);
        break;

    default:
        files = QWebEnginePage::chooseFiles(mode, oldFiles, acceptedMimeTypes);
        break;
    }

    if (!files.isEmpty())
        s_lastUploadLocation = files.at(0);

    return files;
}

QStringList WebPage::autoFillUsernames() const
{
    return m_autoFillUsernames;
}

bool WebPage::javaScriptPrompt(const QUrl &securityOrigin, const QString &msg, const QString &defaultValue, QString* result)
{
    if (!kEnableJsNonBlockDialogs) {
        return QWebEnginePage::javaScriptPrompt(securityOrigin, msg, defaultValue, result);
    }

    if (m_runningLoop) {
        return false;
    }

    ResizableFrame* widget = new ResizableFrame(view()->overlayWidget());

    widget->setObjectName("jsFrame");
    Ui_jsPrompt* ui = new Ui_jsPrompt();
    ui->setupUi(widget);
    ui->message->setText(msg);
    ui->lineEdit->setText(defaultValue);
    ui->lineEdit->setFocus();
    widget->resize(view()->size());
    widget->show();

    connect(view(), SIGNAL(viewportResized(QSize)), widget, SLOT(slotResize(QSize)));
    connect(ui->lineEdit, SIGNAL(returnPressed()), ui->buttonBox->button(QDialogButtonBox::Ok), SLOT(animateClick()));

    QEventLoop eLoop;
    m_runningLoop = &eLoop;
    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), &eLoop, SLOT(quit()));

    if (eLoop.exec() == 1) {
        return result;
    }
    m_runningLoop = 0;

    QString x = ui->lineEdit->text();
    bool _result = ui->buttonBox->clickedButtonRole() == QDialogButtonBox::AcceptRole;
    *result = x;

    delete widget;
    view()->setFocus();

    return _result;
}

bool WebPage::javaScriptConfirm(const QUrl &securityOrigin, const QString &msg)
{
    if (!kEnableJsNonBlockDialogs) {
        return QWebEnginePage::javaScriptConfirm(securityOrigin, msg);
    }

    if (m_runningLoop) {
        return false;
    }

    ResizableFrame* widget = new ResizableFrame(view()->overlayWidget());

    widget->setObjectName("jsFrame");
    Ui_jsConfirm* ui = new Ui_jsConfirm();
    ui->setupUi(widget);
    ui->message->setText(msg);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setFocus();
    widget->resize(view()->size());
    widget->show();

    connect(view(), SIGNAL(viewportResized(QSize)), widget, SLOT(slotResize(QSize)));

    QEventLoop eLoop;
    m_runningLoop = &eLoop;
    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), &eLoop, SLOT(quit()));

    if (eLoop.exec() == 1) {
        return false;
    }
    m_runningLoop = 0;

    bool result = ui->buttonBox->clickedButtonRole() == QDialogButtonBox::AcceptRole;

    delete widget;
    view()->setFocus();

    return result;
}

void WebPage::javaScriptAlert(const QUrl &securityOrigin, const QString &msg)
{
    Q_UNUSED(securityOrigin)

    if (m_blockAlerts || m_runningLoop) {
        return;
    }

    if (!kEnableJsNonBlockDialogs) {
        QString title = tr("JavaScript alert");
        if (!url().host().isEmpty()) {
            title.append(QString(" - %1").arg(url().host()));
        }

        CheckBoxDialog dialog(QMessageBox::Ok, view());
        dialog.setDefaultButton(QMessageBox::Ok);
        dialog.setWindowTitle(title);
        dialog.setText(msg);
        dialog.setCheckBoxText(tr("Prevent this page from creating additional dialogs"));
        dialog.setIcon(QMessageBox::Information);
        dialog.exec();

        m_blockAlerts = dialog.isChecked();
        return;
    }

    ResizableFrame* widget = new ResizableFrame(view()->overlayWidget());

    widget->setObjectName("jsFrame");
    Ui_jsAlert* ui = new Ui_jsAlert();
    ui->setupUi(widget);
    ui->message->setText(msg);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setFocus();
    widget->resize(view()->size());
    widget->show();

    connect(view(), SIGNAL(viewportResized(QSize)), widget, SLOT(slotResize(QSize)));

    QEventLoop eLoop;
    m_runningLoop = &eLoop;
    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), &eLoop, SLOT(quit()));

    if (eLoop.exec() == 1) {
        return;
    }
    m_runningLoop = 0;

    m_blockAlerts = ui->preventAlerts->isChecked();

    delete widget;

    view()->setFocus();
}

void WebPage::javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID)
{
    if (!kEnableJsOutput) {
        return;
    }

    switch (level) {
    case InfoMessageLevel:
        std::cout << "[I] ";
        break;

    case WarningMessageLevel:
        std::cout << "[W] ";
        break;

    case ErrorMessageLevel:
        std::cout << "[E] ";
        break;
    }

    std::cout << qPrintable(sourceID) << ":" << lineNumber << " " << qPrintable(message);
}

QWebEnginePage* WebPage::createWindow(QWebEnginePage::WebWindowType type)
{
    TabbedWebView *tView = qobject_cast<TabbedWebView*>(view());
    BrowserWindow *window = tView ? tView->browserWindow() : mApp->getWindow();

    auto createTab = [=](Qz::NewTabPositionFlags pos) {
        int index = window->tabWidget()->addView(QUrl(), pos);
        TabbedWebView* view = window->weView(index);
        view->setPage(new WebPage);
        if (tView) {
            tView->webTab()->addChildTab(view->webTab());
        }
        // Workaround focus issue when creating tab
        if (pos.testFlag(Qz::NT_SelectedTab)) {
            QPointer<TabbedWebView> pview = view;
            pview->setFocus();
            QTimer::singleShot(100, this, [pview]() {
                if (pview && pview->webTab()->isCurrentTab()) {
                    pview->setFocus();
                }
            });
        }
        return view->page();
    };

    switch (type) {
    case QWebEnginePage::WebBrowserWindow: {
        BrowserWindow *window = mApp->createWindow(Qz::BW_NewWindow);
        WebPage *page = new WebPage;
        window->setStartPage(page);
        return page;
    }

    case QWebEnginePage::WebDialog:
        if (!qzSettings->openPopupsInTabs) {
            PopupWebView* view = new PopupWebView;
            view->setPage(new WebPage);
            PopupWindow* popup = new PopupWindow(view);
            popup->show();
            window->addDeleteOnCloseWidget(popup);
            return view->page();
        }
        // else fallthrough

    case QWebEnginePage::WebBrowserTab:
        return createTab(Qz::NT_CleanSelectedTab);

    case QWebEnginePage::WebBrowserBackgroundTab:
        return createTab(Qz::NT_CleanNotSelectedTab);

    default:
        break;
    }

    return Q_NULLPTR;
}
