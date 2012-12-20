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
#include "siteicon.h"
#include "locationbar.h"
#include "tabbedwebview.h"
#include "globalfunctions.h"

#include <QDrag>
#include <QMimeData>
#include <QApplication>
#include <QContextMenuEvent>

SiteIcon::SiteIcon(LocationBar* parent)
    : ToolButton(parent)
    , m_locationBar(parent)
{
    setObjectName("locationbar-siteicon");
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    setCursor(Qt::ArrowCursor);
    setToolTip(LocationBar::tr("Show information about this page"));
    setFocusPolicy(Qt::ClickFocus);
}

void SiteIcon::contextMenuEvent(QContextMenuEvent* e)
{
    // Prevent propagating to LocationBar
    e->accept();
}

void SiteIcon::mousePressEvent(QMouseEvent* e)
{
    if (e->buttons() & Qt::LeftButton) {
        m_dragStartPosition = e->pos();
    }

    // Prevent propagating to LocationBar
    e->accept();

    ToolButton::mousePressEvent(e);
}

void SiteIcon::mouseMoveEvent(QMouseEvent* e)
{
    if (!m_locationBar || !(e->buttons() & Qt::LeftButton)) {
        ToolButton::mouseMoveEvent(e);
        return;
    }

    int manhattanLength = (e->pos() - m_dragStartPosition).manhattanLength();
    if (manhattanLength <= QApplication::startDragDistance()) {
        ToolButton::mouseMoveEvent(e);
        return;
    }

    const QUrl &url = m_locationBar->webView()->url();
    const QString &title = m_locationBar->webView()->title();

    if (url.isEmpty() || title.isEmpty()) {
        ToolButton::mouseMoveEvent(e);
        return;
    }

    QDrag* drag = new QDrag(this);
    QMimeData* mime = new QMimeData;
    mime->setUrls(QList<QUrl>() << url);
    mime->setText(title);
    mime->setImageData(icon().pixmap(16, 16).toImage());

    drag->setMimeData(mime);
    drag->setPixmap(qz_createPixmapForSite(icon(), title, url.toString()));

    drag->exec();
}
