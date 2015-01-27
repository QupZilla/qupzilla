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
#include "popupwebview.h"
#include "popupwebpage.h"
#include "mainapplication.h"
#include "browserwindow.h"
#include "tabwidget.h"
#include "tabbedwebview.h"
#include "iconprovider.h"
#include "enhancedmenu.h"
#include "loadrequest.h"

#include <QContextMenuEvent>

PopupWebView::PopupWebView(QWidget* parent)
    : WebView(parent)
    , m_page(0)
    , m_menu(new Menu(this))
{
    m_menu->setCloseOnMiddleClick(true);
}

void PopupWebView::setWebPage(PopupWebPage* page)
{
    if (m_page == page) {
        return;
    }

    if (m_page) {
        delete m_page;
        m_page = 0;
    }

    m_page = page;
    m_page->setParent(this);
    setPage(m_page);
}

PopupWebPage* PopupWebView::webPage()
{
    return m_page;
}

QWidget* PopupWebView::overlayWidget()
{
    return this;
}

void PopupWebView::loadInNewTab(const LoadRequest &req, Qz::NewTabPositionFlags position)
{
    Q_UNUSED(position)

    BrowserWindow* window = mApp->getWindow();

    if (window) {
        int index = window->tabWidget()->addView(QUrl(), Qz::NT_SelectedTab);
        window->weView(index)->load(req);
        window->raise();
    }
}

void PopupWebView::openNewTab(Qz::NewTabPositionFlags position)
{
    Q_UNUSED(position)

    BrowserWindow* window = mApp->getWindow();

    if (window) {
        window->tabWidget()->addView(QUrl(), Qz::NT_SelectedTab);
        window->raise();
    }
}

void PopupWebView::closeView()
{
    parentWidget()->close();
}

void PopupWebView::inspectElement()
{
#if QTWEBENGINE_DISABLED
    triggerPageAction(QWebEnginePage::InspectElement);
#endif
}

void PopupWebView::contextMenuEvent(QContextMenuEvent* event)
{
    m_menu->clear();

#if QTWEBENGINE_DISABLED
    const QWebHitTestResult hitTest = page()->mainFrame()->hitTestContent(event->pos());

    createContextMenu(m_menu, hitTest, event->pos());

    m_menu->addSeparator();
    m_menu->addAction(tr("Inspect Element"), this, SLOT(inspectElement()));

    if (!m_menu->isEmpty()) {
        // Prevent choosing first option with double rightclick
        const QPoint pos = event->globalPos();
        QPoint p(pos.x(), pos.y() + 1);
        m_menu->popup(p);
        return;
    }
#endif

    WebView::contextMenuEvent(event);
}
