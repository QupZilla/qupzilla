/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2016  S. Razi Alavizadeh <s.r.alavizadeh@gmail.com>
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
#ifndef TABFILTERDELEGATE_H
#define TABFILTERDELEGATE_H

#include <QStyledItemDelegate>

class TabFilterDelegate : public QStyledItemDelegate
{
public:
    explicit TabFilterDelegate(QObject* parent = 0);

    void paint(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    void viewItemDrawText(QPainter *p, const QStyleOptionViewItem *option, const QRect &rect,
                          const QString &text, const QColor &color,
                          const QString &searchText = QString()) const;

    QString m_filterText;

};

#endif // TABFILTERDELEGATE_H
