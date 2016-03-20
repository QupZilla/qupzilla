/* ============================================================
* GreaseMonkey plugin for QupZilla
* Copyright (C) 2012  David Rosca <nowrep@gmail.com>
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
#ifndef GM_SETTINGSLISTDELEGATE_H
#define GM_SETTINGSLISTDELEGATE_H

#include <QStyledItemDelegate>

class GM_SettingsListDelegate : public QStyledItemDelegate
{
public:
    explicit GM_SettingsListDelegate(QObject* parent = 0);

    int padding() const;

    void paint(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    QPixmap m_removePixmap;
    QIcon m_updateIcon;

    mutable int m_rowHeight;
    mutable int m_padding;
};

#endif // GM_SETTINGSLISTDELEGATE_H
