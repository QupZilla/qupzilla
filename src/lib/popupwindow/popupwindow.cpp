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
#include "popupwindow.h"
#include "popupwebview.h"
#include "webpage.h"
#include "popupstatusbarmessage.h"
#include "progressbar.h"
#include "searchtoolbar.h"
#include "qzsettings.h"
#include "popuplocationbar.h"
#include "qztools.h"
#include "mainapplication.h"
#include "browserwindow.h"

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

    QWidget *locationWidget = new QWidget(this);
    QVBoxLayout *llayout = new QVBoxLayout();
    llayout->setContentsMargins(3, 3, 3, 5);
    llayout->addWidget(m_locationBar);
    locationWidget->setLayout(llayout);

    m_statusBar = new QStatusBar(this);
    m_statusBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    m_progressBar = new ProgressBar(m_statusBar);
    m_statusBar->addPermanentWidget(m_progressBar);
    m_progressBar->hide();

    m_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_statusBarMessage = new PopupStatusBarMessage(this);

    m_notificationWidget = new QWidget(this);
    m_notificationWidget->setAutoFillBackground(true);
    QPalette pal = m_notificationWidget->palette();
    pal.setColor(QPalette::Background, pal.window().color().darker(110));
    m_notificationWidget->setPalette(pal);

    QVBoxLayout *nlayout = new QVBoxLayout(m_notificationWidget);
    nlayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    nlayout->setContentsMargins(0, 0, 0, 0);
    nlayout->setSpacing(1);

    QWidget *viewSpacer = new QWidget(this);
    pal = viewSpacer->palette();
    pal.setColor(QPalette::Background, pal.window().color().darker(125));
    viewSpacer->setPalette(pal);
    viewSpacer->setFixedHeight(1);
    viewSpacer->setAutoFillBackground(true);

    m_menuBar = new QMenuBar(this);

    QMenu* menuFile = new QMenu(tr("File"));
    menuFile->addAction(QIcon::fromTheme("mail-message-new"), tr("Send Link..."), m_view, SLOT(sendPageByMail()));
    menuFile->addAction(QIcon::fromTheme("document-print"), tr("&Print..."), m_view, SLOT(printPage()))->setShortcut(QKeySequence("Ctrl+P"));
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

    QVBoxLayout *l = new QVBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(0);
    l->addWidget(m_view);

    QWidget *viewWidget = new QWidget(this);
    viewWidget->setLayout(l);

    m_layout->insertWidget(0, m_menuBar);
    m_layout->addWidget(locationWidget);
    m_layout->addWidget(viewSpacer);
    m_layout->addWidget(viewWidget);
    m_layout->addWidget(m_statusBar);
    setLayout(m_layout);

    connect(m_view, &WebView::showNotification, this, &PopupWindow::showNotification);
    connect(m_view, &WebView::titleChanged, this, &PopupWindow::titleChanged);
    connect(m_view, &WebView::urlChanged, m_locationBar, &PopupLocationBar::showUrl);
    connect(m_view, &WebView::iconChanged, m_locationBar, &PopupLocationBar::showSiteIcon);
    connect(m_view, &WebView::loadStarted, this, &PopupWindow::loadStarted);
    connect(m_view, &WebView::loadProgress, this, &PopupWindow::loadProgress);
    connect(m_view, &WebView::loadFinished, this, &PopupWindow::loadFinished);

    auto pageChanged = [this](WebPage *page) {
        connect(page, &WebPage::linkHovered, this, &PopupWindow::showStatusBarMessage);
        connect(page, &WebPage::geometryChangeRequested, this, &PopupWindow::setWindowGeometry);
    };
    pageChanged(m_view->page());
    connect(m_view, &WebView::pageChanged, this, pageChanged);

    m_view->setFocus();
    titleChanged();

    QUrl urlToShow = m_view->url();
    if (urlToShow.isEmpty()) {
        urlToShow = m_view->page()->requestedUrl();
    }

    m_locationBar->showUrl(urlToShow);

    if (mApp->getWindow()) {
        m_statusBar->setVisible(mApp->getWindow()->statusBar()->isVisible());
        m_menuBar->setVisible(mApp->getWindow()->menuBar()->isVisible());

        if (m_menuBar->isHidden())
            m_layout->setContentsMargins(0, 2, 0, 0);
    }

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
    m_notificationWidget->setParent(nullptr);
    m_notificationWidget->setParent(m_view->overlayWidget());
    m_notificationWidget->setFixedWidth(m_view->width());
    m_notificationWidget->layout()->addWidget(notif);
    m_notificationWidget->show();
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

void PopupWindow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    m_notificationWidget->setFixedWidth(m_view->width());
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
    if (!Settings().value("allowJavaScriptGeometryChange", true).toBool())
        return;

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
