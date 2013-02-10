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
#include "popupwindow.h"
#include "popupwebview.h"
#include "popupwebpage.h"
#include "popupstatusbarmessage.h"
#include "progressbar.h"
#include "pagescreen.h"
#include "searchtoolbar.h"
#include "qzsettings.h"
#include "popuplocationbar.h"
#include "qztools.h"
#include "iconprovider.h"

#include <QVBoxLayout>
#include <QStatusBar>
#include <QWebFrame>
#include <QCloseEvent>
#include <QMenuBar>

PopupWindow::PopupWindow(PopupWebView* view)
    : QWidget()
    , m_view(view)
    , m_page(qobject_cast<PopupWebPage*>(view->page()))
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
    menuFile->addAction(QIcon::fromTheme("document-print"), tr("&Print..."), m_view, SLOT(printPage()))->setShortcut(QKeySequence("Ctrl+P"));
    menuFile->addSeparator();
    menuFile->addAction(QIcon::fromTheme("window-close"), tr("Close"), this, SLOT(close()))->setShortcut(QKeySequence("Ctrl+W"));
    m_menuBar->addMenu(menuFile);

    m_menuEdit = new QMenu(tr("Edit"));
    m_menuEdit->addAction(QIcon::fromTheme("edit-undo"), tr("&Undo"), this, SLOT(editUndo()))->setShortcut(QKeySequence("Ctrl+Z"));
    m_menuEdit->addAction(QIcon::fromTheme("edit-redo"), tr("&Redo"), this, SLOT(editRedo()))->setShortcut(QKeySequence("Ctrl+Shift+Z"));
    m_menuEdit->addSeparator();
    m_menuEdit->addAction(QIcon::fromTheme("edit-cut"), tr("&Cut"), this, SLOT(editCut()))->setShortcut(QKeySequence("Ctrl+X"));
    m_menuEdit->addAction(QIcon::fromTheme("edit-copy"), tr("C&opy"), this, SLOT(editCopy()))->setShortcut(QKeySequence("Ctrl+C"));
    m_menuEdit->addAction(QIcon::fromTheme("edit-paste"), tr("&Paste"), this, SLOT(editPaste()))->setShortcut(QKeySequence("Ctrl+V"));
    m_menuEdit->addSeparator();
    m_menuEdit->addAction(QIcon::fromTheme("edit-select-all"), tr("Select All"), m_view, SLOT(selectAll()))->setShortcut(QKeySequence("Ctrl+A"));
    m_menuEdit->addAction(QIcon::fromTheme("edit-find"), tr("Find"), this, SLOT(searchOnPage()))->setShortcut(QKeySequence("Ctrl+F"));
    connect(m_menuEdit, SIGNAL(aboutToShow()), this, SLOT(aboutToShowEditMenu()));
    connect(m_menuEdit, SIGNAL(aboutToHide()), this, SLOT(aboutToHideEditMenu()));
    m_menuBar->addMenu(m_menuEdit);

    m_menuView = new QMenu(tr("View"));
    m_actionStop = m_menuView->addAction(qIconProvider->standardIcon(QStyle::SP_BrowserStop), tr("&Stop"), m_view, SLOT(stop()));
    m_actionStop->setShortcut(QKeySequence("Esc"));
    m_actionReload = m_menuView->addAction(qIconProvider->standardIcon(QStyle::SP_BrowserReload), tr("&Reload"), m_view, SLOT(reload()));
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
    foreach(QAction * action, actions) {
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

    aboutToHideEditMenu();

    connect(m_view, SIGNAL(showNotification(QWidget*)), this, SLOT(showNotification(QWidget*)));
    connect(m_view, SIGNAL(titleChanged(QString)), this, SLOT(titleChanged()));
    connect(m_view, SIGNAL(urlChanged(QUrl)), m_locationBar, SLOT(showUrl(QUrl)));
    connect(m_view, SIGNAL(iconChanged()), m_locationBar, SLOT(showSiteIcon()));
    connect(m_view, SIGNAL(statusBarMessage(QString)), this, SLOT(showStatusBarMessage(QString)));
    connect(m_view, SIGNAL(loadStarted()), this, SLOT(loadStarted()));
    connect(m_view, SIGNAL(loadProgress(int)), this, SLOT(loadProgress(int)));
    connect(m_view, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished()));

    connect(m_page, SIGNAL(linkHovered(QString, QString, QString)), this, SLOT(showStatusBarMessage(QString)));
    connect(m_page, SIGNAL(geometryChangeRequested(QRect)), this, SLOT(setWindowGeometry(QRect)));
    connect(m_page, SIGNAL(statusBarVisibilityChangeRequested(bool)), this, SLOT(setStatusBarVisibility(bool)));
    connect(m_page, SIGNAL(menuBarVisibilityChangeRequested(bool)), this, SLOT(setMenuBarVisibility(bool)));
    connect(m_page, SIGNAL(toolBarVisibilityChangeRequested(bool)), this, SLOT(setToolBarVisibility(bool)));

    m_view->setFocus();
    titleChanged();

    QUrl urlToShow = m_view->url();
    if (urlToShow.isEmpty()) {
        urlToShow = m_view->page()->mainFrame()->requestedUrl();
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
    if (m_page->isRunningLoop()) {
        event->ignore();
        return;
    }

    m_page->disconnectObjects();
    m_view->deleteLater();

    event->accept();
}

void PopupWindow::editSelectAll()
{
    m_view->selectAll();
}

void PopupWindow::aboutToShowEditMenu()
{
    m_menuEdit->actions().at(0)->setEnabled(m_view->pageAction(QWebPage::Undo)->isEnabled());
    m_menuEdit->actions().at(1)->setEnabled(m_view->pageAction(QWebPage::Redo)->isEnabled());
    // Separator
    m_menuEdit->actions().at(3)->setEnabled(m_view->pageAction(QWebPage::Cut)->isEnabled());
    m_menuEdit->actions().at(4)->setEnabled(m_view->pageAction(QWebPage::Copy)->isEnabled());
    m_menuEdit->actions().at(5)->setEnabled(m_view->pageAction(QWebPage::Paste)->isEnabled());
    // Separator
    m_menuEdit->actions().at(7)->setEnabled(m_view->pageAction(QWebPage::SelectAll)->isEnabled());
}

void PopupWindow::aboutToHideEditMenu()
{
    m_menuEdit->actions().at(0)->setEnabled(false);
    m_menuEdit->actions().at(1)->setEnabled(false);
    // Separator
    m_menuEdit->actions().at(3)->setEnabled(false);
    m_menuEdit->actions().at(4)->setEnabled(false);
    m_menuEdit->actions().at(5)->setEnabled(false);
    // Separator
    m_menuEdit->actions().at(7)->setEnabled(false);
}

void PopupWindow::savePageScreen()
{
    PageScreen* pageScreen = new PageScreen(m_view, this);
    pageScreen->show();
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

void PopupWindow::editUndo()
{
    m_view->triggerPageAction(QWebPage::Undo);
}

void PopupWindow::editRedo()
{
    m_view->triggerPageAction(QWebPage::Redo);
}

void PopupWindow::editCut()
{
    m_view->triggerPageAction(QWebPage::Cut);
}

void PopupWindow::editCopy()
{
    m_view->triggerPageAction(QWebPage::Copy);
}

void PopupWindow::editPaste()
{
    m_view->triggerPageAction(QWebPage::Paste);
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
