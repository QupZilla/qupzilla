/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2013  David Rosca <nowrep@gmail.com>
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
#include "horizontallistwidget.h"

#include <QMouseEvent>

HorizontalListWidget::HorizontalListWidget(QWidget* parent)
    : QListWidget(parent)
    , m_mouseDown(false)
{
    setFocusPolicy(Qt::NoFocus);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setMovement(QListView::Static);
    setResizeMode(QListView::Adjust);
    setViewMode(QListView::IconMode);
    setSelectionRectVisible(false);
}

void HorizontalListWidget::mousePressEvent(QMouseEvent* event)
{
    m_mouseDown = true;

    QListWidget::mousePressEvent(event);
}

void HorizontalListWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (!itemAt(event->pos())) {
        // Don't unselect item so it ends up with no item selected
        return;
    }

    QListWidget::mouseMoveEvent(event);
}

void HorizontalListWidget::mouseReleaseEvent(QMouseEvent* event)
{
    m_mouseDown = false;

    QListWidget::mouseReleaseEvent(event);
}

void HorizontalListWidget::wheelEvent(QWheelEvent* event)
{
    // As this is just Horizontal ListWidget, disable wheel scrolling completely
    Q_UNUSED(event)
}
