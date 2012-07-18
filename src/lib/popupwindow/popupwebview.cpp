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
#include "popupwebview.h"
#include "popupwebpage.h"
#include "mainapplication.h"
#include "qupzilla.h"
#include "tabwidget.h"
#include "iconprovider.h"
#include "enhancedmenu.h"

#include <QWebFrame>

PopupWebView::PopupWebView(QWidget* parent)
    : WebView(parent)
    , m_page(0)
    , m_menu(new Menu(this))
{
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

QWidget* PopupWebView::overlayForJsAlert()
{
    return this;
}

void PopupWebView::openUrlInNewTab(const QUrl &urla, Qz::NewTabPositionFlag position)
{
    Q_UNUSED(position)

    QupZilla* window = mApp->getWindow();

    if (window) {
        QNetworkRequest req(urla);
        req.setRawHeader("Referer", url().toEncoded());
        req.setRawHeader("X-QupZilla-UserLoadAction", QByteArray("1"));

        window->tabWidget()->addView(req, Qz::NT_SelectedTab);
        window->raise();
    }
}

void PopupWebView::closeView()
{
    parentWidget()->close();
}

void PopupWebView::contextMenuEvent(QContextMenuEvent* event)
{
    m_menu->clear();

    const QWebHitTestResult &hitTest = page()->mainFrame()->hitTestContent(event->pos());

    createContextMenu(m_menu, hitTest, event->pos());

    if (!m_menu->isEmpty()) {
        //Prevent choosing first option with double rightclick
        const QPoint &pos = event->globalPos();
        QPoint p(pos.x(), pos.y() + 1);
        m_menu->popup(p);
        return;
    }

    WebView::contextMenuEvent(event);
}
