/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2013-2018 David Rosca <nowrep@gmail.com>
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
#include "navigationcontainer.h"
#include "qzsettings.h"
#include "tabbar.h"

#include <QPainter>
#include <QVBoxLayout>

NavigationContainer::NavigationContainer(QWidget* parent)
    : QWidget(parent)
    , m_tabBar(0)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    setLayout(m_layout);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
}

void NavigationContainer::addWidget(QWidget* widget)
{
    m_layout->addWidget(widget);
}

void NavigationContainer::setTabBar(TabBar* tabBar)
{
    m_tabBar = tabBar;
    m_layout->addWidget(m_tabBar);

    toggleTabsOnTop(qzSettings->tabsOnTop);
}

void NavigationContainer::toggleTabsOnTop(bool enable)
{
    setUpdatesEnabled(false);

    m_layout->removeWidget(m_tabBar);
    m_layout->insertWidget(enable ? 0 : m_layout->count(), m_tabBar);
    m_layout->setContentsMargins(0, enable ? 2 : 0, 0, enable ? 2 : 0);

    setUpdatesEnabled(true);
}

void NavigationContainer::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);

    // Draw line at the bottom of navigation bar if tabs are on top
    // To visually distinguish navigation bar from the page

    if (qzSettings->tabsOnTop) {
        QPainter p(this);
        QRect lineRect(0, height() - 1, width(), 1);
        QColor c = palette().window().color().darker(125);
        p.fillRect(lineRect, c);
    }
}
