/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2016 David Rosca <nowrep@gmail.com>
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

#include "qzcommon.h"

class LocationCompleterView;

class QUPZILLA_EXPORT LocationCompleterDelegate : public QStyledItemDelegate
{
public:
    explicit LocationCompleterDelegate(LocationCompleterView* parent = 0);

    void paint(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void setShowSwitchToTab(bool enable);

private:
    bool drawSwitchToTab() const;

    void viewItemDrawText(QPainter *p, const QStyleOptionViewItem *option, const QRect &rect,
                          const QString &text, const QColor &color,
                          const QString &searchText = QString()) const;

    mutable int m_rowHeight;
    mutable int m_padding;
    bool m_drawSwitchToTab;

    LocationCompleterView* m_view;
};

#endif // LOCATIONCOMPLETERDELEGATE_H
