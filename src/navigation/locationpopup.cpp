/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  nowrep
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
#include "locationpopup.h"
LocationPopup::LocationPopup(QWidget* parent)
   :QAbstractItemView()
   ,m_parent(parent)
{
    setWindowFlags(Qt::Popup);
}

void LocationPopup::show()
{
    QPoint p = m_parent->mapToGlobal(QPoint(0, 0));
    move( (p.x() ), (p.y() + m_parent->height()));
    resize(m_parent->width(), 100);
    QAbstractItemView::show();
}

#if 0
QRect LocationPopup::visualRect(const QModelIndex &index) const
{

}

void LocationPopup::scrollTo(const QModelIndex &index, ScrollHint hint)
{

}

QModelIndex LocationPopup::indexAt(const QPoint &point) const
{

}

QModelIndex LocationPopup::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{

}

int LocationPopup::horizontalOffset() const
{

}

int LocationPopup::verticalOffset() const
{

}

bool LocationPopup::isIndexHidden(const QModelIndex &index) const
{

}

void LocationPopup::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{

}

QRegion LocationPopup::visualRegionForSelection(const QItemSelection &selection) const
{

}
#endif
