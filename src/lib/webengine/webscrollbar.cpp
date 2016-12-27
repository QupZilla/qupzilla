/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2016 David Rosca <nowrep@gmail.com>
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

#include "webscrollbar.h"
#include "webview.h"
#include "webpage.h"

#include <QPaintEvent>

WebScrollBar::WebScrollBar(Qt::Orientation orientation, WebView *view)
    : QScrollBar(orientation)
    , m_view(view)
{
    setFocusProxy(m_view);
    resize(sizeHint());

    connect(this, &QScrollBar::valueChanged, this, &WebScrollBar::performScroll);
    connect(view, &WebView::focusChanged, this, [this]() { update(); });
}

int WebScrollBar::thickness() const
{
    return orientation() == Qt::Vertical ? width() : height();
}

void WebScrollBar::updateValues(const QSize &viewport)
{
    setMinimum(0);
    setParent(m_view->overlayWidget());

    int newValue;

    if (orientation() == Qt::Vertical) {
        setFixedHeight(viewport.height());
        move(m_view->width() - width(), 0);
        setPageStep(viewport.height());
        setMaximum(std::max(0, m_view->page()->contentsSize().toSize().height() - viewport.height()));
        newValue = m_view->page()->scrollPosition().toPoint().y();
    } else {
        setFixedWidth(viewport.width());
        move(0, m_view->height() - height());
        setPageStep(viewport.width());
        setMaximum(std::max(0, m_view->page()->contentsSize().toSize().width() - viewport.width()));
        newValue = m_view->page()->scrollPosition().toPoint().x();
    }

    if (!isSliderDown()) {
        m_blockScrolling = true;
        setValue(newValue);
        m_blockScrolling = false;
    }

    setVisible(maximum() > minimum());
}

void WebScrollBar::performScroll()
{
    if (m_blockScrolling) {
        return;
    }

    QPointF pos = m_view->page()->scrollPosition();

    if (orientation() == Qt::Vertical) {
        pos.setY(value());
    } else {
        pos.setX(value());
    }

    m_view->page()->setScrollPosition(pos);
}

void WebScrollBar::paintEvent(QPaintEvent *ev)
{
    QPainter painter(this);
    painter.fillRect(ev->rect(), m_view->page()->backgroundColor());
    QScrollBar::paintEvent(ev);
}
