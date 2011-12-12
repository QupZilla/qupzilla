/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca
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
#include "buttonwithmenu.h"

ButtonWithMenu::ButtonWithMenu(QWidget* parent)
    : ToolButton(parent)
    , m_menu(new QMenu(this))
    , m_currentItem(0)
{
    setPopupMode(QToolButton::InstantPopup);
    setCursor(Qt::ArrowCursor);
    setFocusPolicy(Qt::ClickFocus);
    setMenu(m_menu);

    connect(m_menu, SIGNAL(aboutToShow()), this, SLOT(generateMenu()));
}

void ButtonWithMenu::setCurrentItem()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        setCurrentItem(action->data().value<Item>());
    }
}

void ButtonWithMenu::clearItems()
{
    m_menu->clear();
    m_items.clear();

    m_currentItem = 0;
}

void ButtonWithMenu::addItem(const Item &item)
{
    m_items.append(item);

    if (!m_currentItem) {
        setCurrentItem(item);
    }

    emit itemAdded(item);
}

void ButtonWithMenu::addItems(const QList<Item> &items)
{
    foreach(const Item & item, items) {
        addItem(item);
    }
}

void ButtonWithMenu::removeItem(const Item &item)
{
    int index = m_items.indexOf(item);
    if (index < 0) {
        return;
    }

    m_items.removeOne(item);

    if (m_items.count() == 0) {
        setIcon(QIcon());
        return;
    }

    if (*m_currentItem == item) {
        setCurrentItem(m_items.first());
    }
}

void ButtonWithMenu::setCurrentItem(const Item &item)
{
    int index = m_items.indexOf(item);
    if (index < 0) {
        return;
    }

    m_currentItem = const_cast<Item*>(&m_items.at(index));

    setIcon(m_currentItem->icon);
    setToolTip(m_currentItem->text);

    emit activeItemChanged(*m_currentItem);
}

void ButtonWithMenu::wheelEvent(QWheelEvent* event)
{
    int currItemIndex = m_items.indexOf(*m_currentItem);
    int itemsCount = m_items.count();

    if (itemsCount == 0) {
        return;
    }

    int numDegrees = event->delta() / 8;
    int numSteps = numDegrees / 15;
    if (numSteps == 1) {
        if (currItemIndex != 0) {
            setCurrentItem(m_items.at(currItemIndex - 1));
        }
    }
    else if (currItemIndex < itemsCount - 1) {
        setCurrentItem(m_items.at(currItemIndex + 1));
    }

    event->accept();
}

ButtonWithMenu::Item* ButtonWithMenu::currentItem()
{
    return m_currentItem;
}

QMenu* ButtonWithMenu::menu() const
{
    return m_menu;
}

void ButtonWithMenu::generateMenu()
{
    m_menu->clear();

    foreach(Item item, m_items) {
        QVariant variant;
        variant.setValue<Item>(item);
        m_menu->addAction(item.icon, item.text, this, SLOT(setCurrentItem()))->setData(variant);
    }
}

ButtonWithMenu::~ButtonWithMenu()
{
}
