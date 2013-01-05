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
#include "popupwebpage.h"
#include "popupwebview.h"
#include "popupwindow.h"
#include "qupzilla.h"
#include "tabwidget.h"
#include "tabbedwebview.h"
#include "qzsettings.h"

#include <QTimer>
#include <QStatusBar>

// Wrapper class to detect whether window is opened from JavaScript window.open method
// It has to be done this way, because QtWebKit has really bad API when it comes to opening
// new windows.
//
// Got an idea how to determine it from kWebKitPart.

PopupWebPage::PopupWebPage(QWebPage::WebWindowType type, QupZilla* mainClass)
    : WebPage(mainClass)
    , p_QupZilla(mainClass)
    , m_type(type)
    , m_createNewWindow(false)
    , m_menuBarVisible(false)
    , m_statusBarVisible(false)
    , m_toolBarVisible(false)
    , m_isLoading(false)
    , m_progress(0)
{
    connect(this, SIGNAL(geometryChangeRequested(QRect)), this, SLOT(slotGeometryChangeRequested(QRect)));
    connect(this, SIGNAL(menuBarVisibilityChangeRequested(bool)), this, SLOT(slotMenuBarVisibilityChangeRequested(bool)));
    connect(this, SIGNAL(toolBarVisibilityChangeRequested(bool)), this, SLOT(slotToolBarVisibilityChangeRequested(bool)));
    connect(this, SIGNAL(statusBarVisibilityChangeRequested(bool)), this, SLOT(slotStatusBarVisibilityChangeRequested(bool)));

    connect(this, SIGNAL(loadStarted()), this, SLOT(slotLoadStarted()));
    connect(this, SIGNAL(loadProgress(int)), this, SLOT(slotLoadProgress(int)));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(slotLoadFinished(bool)));

    QTimer::singleShot(0, this, SLOT(checkBehaviour()));
}

void PopupWebPage::slotGeometryChangeRequested(const QRect &rect)
{
    if (rect.isValid() && qzSettings->allowJsGeometryChange) {
        m_geometry = rect;
        m_createNewWindow = true;
    }
}

void PopupWebPage::slotMenuBarVisibilityChangeRequested(bool visible)
{
    if (!qzSettings->allowJsHideMenuBar) return;
    m_menuBarVisible = visible;
}

void PopupWebPage::slotStatusBarVisibilityChangeRequested(bool visible)
{
    if (!qzSettings->allowJsHideStatusBar) return;
    m_statusBarVisible = visible;
}

void PopupWebPage::slotToolBarVisibilityChangeRequested(bool visible)
{
    if (!qzSettings->allowJsHideToolBar) return;
    m_toolBarVisible = visible;
}

void PopupWebPage::slotLoadStarted()
{
    m_isLoading = true;
    m_progress = 0;
}

void PopupWebPage::slotLoadProgress(int prog)
{
    m_progress = prog;
}

void PopupWebPage::slotLoadFinished(bool state)
{
    Q_UNUSED(state)

    m_isLoading = false;
    m_progress = 0;
}

void PopupWebPage::checkBehaviour()
{
    if (m_createNewWindow) {
        PopupWebView* view = new PopupWebView;
        view->setWebPage(this);

        PopupWindow* popup = new PopupWindow(view, p_QupZilla->statusBar()->isVisible());
        popup->setWindowGeometry(m_geometry);
        popup->setMenuBarVisibility(m_menuBarVisible);
        popup->setStatusBarVisibility(m_statusBarVisible);
        popup->setToolBarVisibility(m_toolBarVisible);
        popup->show();

        if (m_isLoading) {
            view->fakeLoadingProgress(m_progress);
        }

        p_QupZilla->addDeleteOnCloseWidget(popup);

        disconnect(this, SIGNAL(geometryChangeRequested(QRect)), this, SLOT(slotGeometryChangeRequested(QRect)));
        disconnect(this, SIGNAL(menuBarVisibilityChangeRequested(bool)), this, SLOT(slotMenuBarVisibilityChangeRequested(bool)));
        disconnect(this, SIGNAL(toolBarVisibilityChangeRequested(bool)), this, SLOT(slotToolBarVisibilityChangeRequested(bool)));
        disconnect(this, SIGNAL(statusBarVisibilityChangeRequested(bool)), this, SLOT(slotStatusBarVisibilityChangeRequested(bool)));

        disconnect(this, SIGNAL(loadStarted()), this, SLOT(slotLoadStarted()));
        disconnect(this, SIGNAL(loadProgress(int)), this, SLOT(slotLoadProgress(int)));
        disconnect(this, SIGNAL(loadFinished(bool)), this, SLOT(slotLoadFinished(bool)));
    }
    else {
        int index = p_QupZilla->tabWidget()->addView(QUrl(), Qz::NT_CleanSelectedTab);
        TabbedWebView* view = p_QupZilla->weView(index);
        view->setWebPage(this);

        if (m_isLoading) {
            view->fakeLoadingProgress(m_progress);
        }
    }
}
