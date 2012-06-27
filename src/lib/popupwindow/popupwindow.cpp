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
#include "popupwindow.h"
#include "popupwebview.h"
#include "popupwebpage.h"
#include "popupstatusbarmessage.h"
#include "progressbar.h"
#include "popuplocationbar.h"
#include "globalfunctions.h"

#include <QVBoxLayout>
#include <QStatusBar>
#include <QWebFrame>

PopupWindow::PopupWindow(PopupWebView* view, bool showStatusBar)
    : QWidget()
    , m_view(view)
    , m_page(qobject_cast<PopupWebPage*>(view->page()))
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    m_locationBar = new PopupLocationBar(this);
    m_locationBar->setView(m_view);

    m_statusBar = new QStatusBar(this);
    m_statusBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    m_statusBar->setVisible(showStatusBar);

    m_progressBar = new ProgressBar(m_statusBar);
    m_statusBar->addPermanentWidget(m_progressBar);
    m_progressBar->hide();

    m_statusBarMessage = new PopupStatusBarMessage(this);

    m_layout->addWidget(m_locationBar);
    m_layout->addWidget(m_view);
    m_layout->addWidget(m_statusBar);
    setLayout(m_layout);

    connect(m_view, SIGNAL(showNotification(QWidget*)), this, SLOT(showNotification(QWidget*)));
    connect(m_view, SIGNAL(titleChanged(QString)), this, SLOT(titleChanged()));
    connect(m_view, SIGNAL(urlChanged(QUrl)), m_locationBar, SLOT(showUrl(QUrl)));
    connect(m_view, SIGNAL(iconChanged()), m_locationBar, SLOT(showIcon()));
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
    if (m_layout->count() > 3) {
        delete m_layout->itemAt(1)->widget();
    }

    m_layout->insertWidget(1, notif);
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

void PopupWindow::setWindowGeometry(const QRect &newRect)
{
    if (newRect.isValid()) {
        QRect oldRect = rect();
        move(newRect.topLeft());

        QSize newSize = newRect.size();
        int additionalHeight = height() - m_view->height();
        newSize.setHeight(newSize.height() + additionalHeight);
        resize(newSize);

        if (newRect.topLeft() == QPoint(0, 0) && oldRect.topLeft() == QPoint(0, 0)) {
            qz_centerWidgetOnScreen(this);
        }
    }
}

// From my testing, these 3 slots are always fired with false
// visible argument (even if true should be passed)
// So for now, we just do nothing here

void PopupWindow::setStatusBarVisibility(bool visible)
{
    Q_UNUSED(visible)
}

void PopupWindow::setMenuBarVisibility(bool visible)
{
    Q_UNUSED(visible)
}

void PopupWindow::setToolBarVisibility(bool visible)
{
    Q_UNUSED(visible)
}

void PopupWindow::titleChanged()
{
    setWindowTitle(tr("%1 - QupZilla").arg(m_view->title()));
}
