/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2017 David Rosca <nowrep@gmail.com>
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
#ifndef BUTTONWITHMENU_H
#define BUTTONWITHMENU_H

#include <QVariant>

#include "toolbutton.h"
#include "wheelhelper.h"

// Only to be used in WebSearchBar
class ButtonWithMenu : public ToolButton
{
    Q_OBJECT
public:
    struct Item {
        QString text;
        QIcon icon;
        QVariant userData;

        Item(const QString &a = QString(), const QIcon &b = QIcon()) {
            text = a;
            icon = b;
        }

        bool operator==(const Item &a) {
            return (a.text == text) && (a.icon.pixmap(16).toImage() == icon.pixmap(16).toImage());
        }

        bool isEmpty() {
            return (text.isEmpty() &&  icon.isNull());
        }

        void clear() {
            text = QString();
            icon = QIcon();
            userData = QVariant();
        }
    };

    explicit ButtonWithMenu(QWidget* parent = 0);
    ~ButtonWithMenu();

    void addItem(const Item &item);
    void addItems(const QVector<Item> &items);
    void removeItem(const Item &item);
    void setCurrentItem(const Item &item, bool emitSignal = true);
    void setCurrentIndex(int index, bool emitSignal = true);

    Item currentItem();
    QVector<Item> allItems() { return m_items; }
    QMenu* menu() const;

signals:
    void activeItemChanged(const ButtonWithMenu::Item &item);
    void itemAdded(const ButtonWithMenu::Item &item);
    void itemRemoved(const ButtonWithMenu::Item &item);

public slots:
    void clearItems();

    void selectNextItem();
    void selectPreviousItem();

private slots:
    void setCurrentItem();
    void generateMenu();

private:
    void mousePressEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent* event);

    QMenu* m_menu;
    QVector<Item> m_items;
    Item m_currentItem;
    WheelHelper m_wheelHelper;
};

// Hint to QVector to use std::realloc on item moving
Q_DECLARE_TYPEINFO(ButtonWithMenu::Item, Q_MOVABLE_TYPE);

Q_DECLARE_METATYPE(ButtonWithMenu::Item)

#endif // BUTTONWITHMENU_H
