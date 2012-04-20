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
#ifndef LOCATIONCOMPLETERDELEGATE_H
#define LOCATIONCOMPLETERDELEGATE_H

#include <QStyledItemDelegate>
#include <QListView>

#include "qz_namespace.h"

class QT_QUPZILLA_EXPORT CompleterListView : public QListView
{
    Q_OBJECT
public:
    explicit CompleterListView(QWidget* parent = 0);

    bool ignoreSelectedFlag() const;

    int rowHeight() const;
    void setRowHeight(int height);

private slots:
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);
protected:
    void mouseMoveEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent* event);

private:
    bool m_selectedItemByMousePosition;
    int m_rowHeight;

    QModelIndex m_lastMouseIndex;
};

class QT_QUPZILLA_EXPORT LocationCompleterDelegate : public QStyledItemDelegate
{
public:
    explicit LocationCompleterDelegate(CompleterListView* parent = 0);

    void paint(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    mutable int m_rowHeight;
    mutable int m_padding;

    CompleterListView* m_listView;
};

#endif // LOCATIONCOMPLETERDELEGATE_H
