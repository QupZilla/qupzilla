/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2015  David Rosca <nowrep@gmail.com>
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
#include "popupwindow.h"
#include "popupwebview.h"
#include "webpage.h"
#include "popupstatusbarmessage.h"
#include "progressbar.h"
#include "pagescreen.h"
#include "searchtoolbar.h"
#include "qzsettings.h"
#include "popuplocationbar.h"
#include "qztools.h"

#include <QVBoxLayout>
#include <QStatusBar>
#include <QCloseEvent>
#include <QMenuBar>

PopupWindow::PopupWindow(PopupWebView* view)
    : QWidget()
    , m_view(view)
    , m_search(0)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    m_locationBar = new PopupLocationBar(this);
    m_locationBar->setView(m_view);

    m_statusBar = new QStatusBar(this);
    m_statusBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    m_progressBar = new ProgressBar(m_statusBar);
    m_statusBar->addPermanentWidget(m_progressBar);
    m_progressBar->hide();

    m_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_statusBarMessage = new PopupStatusBarMessage(this);

    m_menuBar = new QMenuBar(this);

    QMenu* menuFile = new QMenu(tr("File"));
    menuFile->addAction(QIcon::fromTheme("document-save"), tr("&Save Page As..."), m_view, SLOT(savePageAs()))->setShortcut(QKeySequence("Ctrl+S"));
    menuFile->addAction(tr("Save Page Screen"), this, SLOT(savePageScreen()));
    menuFile->addAction(QIcon::fromTheme("mail-message-new"), tr("Send Link..."), m_view, SLOT(sendPageByMail()));
    //menuFile->addAction(QIcon::fromTheme("document-print"), tr("&Print..."), m_view, SLOT(printPage()))->setShortcut(QKeySequence("Ctrl+P"));
    menuFile->addSeparator();
    menuFile->addAction(QIcon::fromTheme("window-close"), tr("Close"), this, SLOT(close()))->setShortcut(QKeySequence("Ctrl+W"));
    m_menuBar->addMenu(menuFile);

    m_menuEdit = new QMenu(tr("Edit"));
    m_menuEdit->addAction(m_view->pageAction(QWebEnginePage::Undo));
    m_menuEdit->addAction(m_view->pageAction(QWebEnginePage::Redo));
    m_menuEdit->addSeparator();
    m_menuEdit->addAction(m_view->pageAction(QWebEnginePage::Cut));
    m_menuEdit->addAction(m_view->pageAction(QWebEnginePage::Copy));
    m_menuEdit->addAction(m_view->pageAction(QWebEnginePage::Paste));
    m_menuEdit->addSeparator();
    m_menuEdit->addAction(m_view->pageAction(QWebEnginePage::SelectAll));
    m_menuEdit->addAction(QIcon::fromTheme("edit-find"), tr("Find"), this, SLOT(searchOnPage()))->setShortcut(QKeySequence("Ctrl+F"));
    m_menuBar->addMenu(m_menuEdit);

    m_menuView = new QMenu(tr("View"));
    m_actionStop = m_menuView->addAction(QIcon::fromTheme(QSL("process-stop")), tr("&Stop"), m_view, SLOT(stop()));
    m_actionStop->setShortcut(QKeySequence("Esc"));
    m_actionReload = m_menuView->addAction(QIcon::fromTheme(QSL("view-refresh")), tr("&Reload"), m_view, SLOT(reload()));
    m_actionReload->setShortcut(QKeySequence("F5"));
    m_menuView->addSeparator();
    m_menuView->addAction(QIcon::fromTheme("zoom-in"), tr("Zoom &In"), m_view, SLOT(zoomIn()))->setShortcut(QKeySequence("Ctrl++"));
    m_menuView->addAction(QIcon::fromTheme("zoom-out"), tr("Zoom &Out"), m_view, SLOT(zoomOut()))->setShortcut(QKeySequence("Ctrl+-"));
    m_menuView->addAction(QIcon::fromTheme("zoom-original"), tr("Reset"), m_view, SLOT(zoomReset()))->setShortcut(QKeySequence("Ctrl+0"));
    m_menuView->addSeparator();
    m_menuView->addAction(QIcon::fromTheme("text-html"), tr("&Page Source"), m_view, SLOT(showSource()))->setShortcut(QKeySequence("Ctrl+U"));
    m_menuBar->addMenu(m_menuView);

    // Make shortcuts available even with hidden menubar
    QList<QAction*> actions = m_menuBar->actions();
    foreach (QAction* action, actions) {
        if (action->menu()) {
            actions += action->menu()->actions();
        }
        addAction(action);
    }

    m_layout->insertWidget(0, m_menuBar);
    m_layout->addWidget(m_locationBar);
    m_layout->addWidget(m_view);
    m_layout->addWidget(m_statusBar);
    setLayout(m_layout);

    connect(m_view, SIGNAL(showNotification(QWidget*)), this, SLOT(showNotification(QWidget*)));
    connect(m_view, SIGNAL(titleChanged(QString)), this, SLOT(titleChanged()));
    connect(m_view, SIGNAL(urlChanged(QUrl)), m_locationBar, SLOT(showUrl(QUrl)));
    connect(m_view, SIGNAL(iconChanged()), m_locationBar, SLOT(showSiteIcon()));
    //connect(m_view, SIGNAL(statusBarMessage(QString)), this, SLOT(showStatusBarMessage(QString)));
    connect(m_view, SIGNAL(loadStarted()), this, SLOT(loadStarted()));
    connect(m_view, SIGNAL(loadProgress(int)), this, SLOT(loadProgress(int)));
    connect(m_view, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished()));

    connect(m_view->page(), &WebPage::linkHovered, this, &PopupWindow::showStatusBarMessage);
    connect(m_view->page(), &WebPage::geometryChangeRequested, this, &PopupWindow::setWindowGeometry);

    m_view->setFocus();
    titleChanged();

    QUrl urlToShow = m_view->url();
    if (urlToShow.isEmpty()) {
        urlToShow = m_view->page()->requestedUrl();
    }

    m_locationBar->showUrl(urlToShow);

    // Ensuring correct sizes for widgets in layout are calculated even
    // before calling QWidget::show()
    m_layout->invalidate();
    m_layout->activate();
}

QStatusBar* PopupWindow::statusBar()
{
    return m_statusBar;
}

PopupWebView* PopupWindow::webView()
{
    return m_view;
}

void PopupWindow::showNotification(QWidget* notif)
{
    if (m_layout->count() > 4) {
        delete m_layout->itemAt(2)->widget();
    }

    m_layout->insertWidget(2, notif);
    notif->show();
}

void PopupWindow::showStatusBarMessage(const QString &message)
{
    if (message.isEmpty()) {
        m_statusBarMessage->clearMessage();
    }
    else {
        m_statusBarMessage->showMessage(message);
    }
}

void PopupWindow::loadStarted()
{
    m_progressBar->setValue(0);
    m_progressBar->show();

    m_locationBar->startLoading();

    if (m_actionStop) {
        m_actionStop->setEnabled(true);
        m_actionReload->setEnabled(false);
    }
}

void PopupWindow::loadProgress(int value)
{
    m_progressBar->show();
    m_progressBar->setValue(value);
}

void PopupWindow::loadFinished()
{
    m_progressBar->hide();

    m_locationBar->stopLoading();

    if (m_actionStop) {
        m_actionStop->setEnabled(false);
        m_actionReload->setEnabled(true);
    }
}

void PopupWindow::closeEvent(QCloseEvent* event)
{
    if (m_view->page()->isRunningLoop()) {
        event->ignore();
        return;
    }

    m_view->deleteLater();

    event->accept();
}

void PopupWindow::savePageScreen()
{
#if QTWEBENGINE_DISABLED
    PageScreen* pageScreen = new PageScreen(m_view, this);
    pageScreen->show();
#endif
}

void PopupWindow::searchOnPage()
{
    if (!m_search) {
        m_search = new SearchToolBar(m_view, this);
        m_search.data()->showMinimalInPopupWindow();
        m_layout->insertWidget(m_layout->count() - 1, m_search);
    }

    m_search->focusSearchLine();
}

void PopupWindow::titleChanged()
{
    setWindowTitle(tr("%1 - QupZilla").arg(m_view->title()));
}

void PopupWindow::setWindowGeometry(QRect newRect)
{
    if (!qzSettings->allowJsGeometryChange) {
        return;
    }

    // left/top was set while width/height not
    if (!newRect.topLeft().isNull() && newRect.size().isNull()) {
        newRect.setSize(QSize(550, 585));
    }

    if (newRect.isValid()) {
        QRect oldRect = rect();
        move(newRect.topLeft());

        QSize newSize = newRect.size();
        int additionalHeight = height() - m_view->height();
        newSize.setHeight(newSize.height() + additionalHeight);
        resize(newSize);

        if (newRect.topLeft() == QPoint(0, 0) && oldRect.topLeft() == QPoint(0, 0)) {
            QzTools::centerWidgetOnScreen(this);
        }
    }
}

void PopupWindow::setStatusBarVisibility(bool visible)
{
    m_statusBar->setVisible(qzSettings->allowJsHideStatusBar ? visible : true);
}

void PopupWindow::setMenuBarVisibility(bool visible)
{
    m_menuBar->setVisible(qzSettings->allowJsHideMenuBar ? visible : true);
}

void PopupWindow::setToolBarVisibility(bool visible)
{
    // Does nothing now
    // m_toolBar->setVisible(qzSettings->allowJsHideToolBar ? visible : true);
    Q_UNUSED(visible)
}
