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
#include "rssicon.h"
#include "rsswidget.h"

#include <QMouseEvent>

RssIcon::RssIcon(QWidget* parent)
    : ClickableLabel(parent)
    , m_view(0)
{
    setObjectName("locationbar-rss-icon");
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::ClickFocus);
    setToolTip(tr("Add RSS from this page..."));

    connect(this, SIGNAL(clicked(QPoint)), this, SLOT(iconClicked()));
}

void RssIcon::setWebView(WebView* view)
{
    m_view = view;
}

void RssIcon::iconClicked()
{
    if (!m_view) {
        return;
    }

    RSSWidget* rss = new RSSWidget(m_view, parentWidget());
    rss->showAt(parentWidget());
}

void RssIcon::contextMenuEvent(QContextMenuEvent* ev)
{
    // Prevent propagating to LocationBar
    ev->accept();
}

void RssIcon::mousePressEvent(QMouseEvent* ev)
{
    ClickableLabel::mousePressEvent(ev);

    // Prevent propagating to LocationBar
    ev->accept();
}
