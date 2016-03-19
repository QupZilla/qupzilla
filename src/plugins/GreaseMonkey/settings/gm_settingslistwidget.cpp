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
#include "gm_settingslistwidget.h"
#include "gm_settingslistdelegate.h"

#include <QMouseEvent>

GM_SettingsListWidget::GM_SettingsListWidget(QWidget* parent)
    : QListWidget(parent)
    , m_delegate(new GM_SettingsListDelegate(this))
{
    // forced to LTR, see issue#967
    setLayoutDirection(Qt::LeftToRight);
    setItemDelegate(m_delegate);
}

void GM_SettingsListWidget::mousePressEvent(QMouseEvent* event)
{
    if (containsRemoveIcon(event->pos())) {
        emit removeItemRequested(itemAt(event->pos()));
        return;
    }

    if (containsUpdateIcon(event->pos())) {
        emit updateItemRequested(itemAt(event->pos()));
        return;
    }

    QListWidget::mousePressEvent(event);
}

void GM_SettingsListWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (containsRemoveIcon(event->pos()) || containsUpdateIcon(event->pos()))
        return;

    QListWidget::mouseDoubleClickEvent(event);
}

bool GM_SettingsListWidget::containsRemoveIcon(const QPoint &pos) const
{
    QListWidgetItem* item = itemAt(pos);
    if (!item) {
        return false;
    }

    const QRect rect = visualItemRect(item);
    const int removeIconPosition = rect.right() - m_delegate->padding() - 16;
    const int center = rect.height() / 2 + rect.top();
    const int removeIconYPos = center - (16 / 2);

    QRect removeIconRect(removeIconPosition, removeIconYPos, 16, 16);

    return removeIconRect.contains(pos);
}

bool GM_SettingsListWidget::containsUpdateIcon(const QPoint &pos) const
{
    QListWidgetItem *item = itemAt(pos);
    if (!item || !item->data(Qt::UserRole + 2).toBool())
        return false;

    const QRect rect = visualItemRect(item);
    const int updateIconPosition = rect.right() - m_delegate->padding() * 2 - 16 * 2;
    const int center = rect.height() / 2 + rect.top();
    const int updateIconYPos = center - (16 / 2);

    QRect updateIconRect(updateIconPosition, updateIconYPos, 16, 16);

    return updateIconRect.contains(pos);
}
