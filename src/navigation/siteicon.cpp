/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca <nowrep@gmail.com>
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
#include "webview.h"

SiteIcon::SiteIcon(LocationBar* parent)
    : ToolButton(parent)
    , m_locationBar(parent)
{
    setObjectName("locationbar-siteicon");
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    setCursor(Qt::ArrowCursor);
    setToolTip(tr("Show informations about this page"));
    setFocusPolicy(Qt::ClickFocus);
}

void SiteIcon::mousePressEvent(QMouseEvent* e)
{
    if (e->buttons() & Qt::LeftButton) {
        m_dragStartPosition = mapFromGlobal(e->globalPos());
    }

    ToolButton::mousePressEvent(e);
}

void SiteIcon::mouseMoveEvent(QMouseEvent* e)
{
    int manhattanLength = (e->pos() - m_dragStartPosition).manhattanLength();
    if (manhattanLength > QApplication::startDragDistance()) {
        ToolButton::mouseMoveEvent(e);
        return;
    }

    QDrag* drag = new QDrag(this);
    QMimeData* mime = new QMimeData;
    mime->setUrls(QList<QUrl>() << m_locationBar->webView()->url());
    mime->setText(m_locationBar->webView()->title());
    mime->setImageData(QVariant::fromValue(icon()));

    drag->setMimeData(mime);
    drag->setPixmap(icon().pixmap(16, 16));

    drag->exec();
}
