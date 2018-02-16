/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018 David Rosca <nowrep@gmail.com>
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
#include "proxystyle.h"
#include "combotabbar.h"

#include <QPainter>
#include <QStyleOption>

ProxyStyle::ProxyStyle()
    : QProxyStyle()
    , m_TabBarTabHSpace(-1)
{
}

int ProxyStyle::styleHint(StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* returnData) const
{
    switch (hint) {
    case QStyle::SH_Menu_Scrollable:
        return int(true);

    case QStyle::SH_TabBar_Alignment:
        return Qt::AlignLeft;

    case QStyle::SH_TabBar_CloseButtonPosition:
        if (qobject_cast<const TabBarHelper*>(widget)) {
            return QTabBar::RightSide;
        }
        break;

    default:
        break;
    }

    return QProxyStyle::styleHint(hint, option, widget, returnData);
}

int ProxyStyle::pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const
{
    switch (metric) {
    case PM_TabBarTabHSpace:
        if (m_TabBarTabHSpace == -1) {
            m_TabBarTabHSpace = qMin(QProxyStyle::pixelMetric(PM_TabBarTabHSpace, option, widget), 14);
        }
        return m_TabBarTabHSpace;

    default:
        return QProxyStyle::pixelMetric(metric, option, widget);
    }
}

void ProxyStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (element == PE_FrameTabBarBase) {
        TabBarHelper *tabBar = qobject_cast<TabBarHelper*>(option->styleObject);
        if (tabBar && tabBar->baseColor().isValid()) {
            painter->setPen(QPen(tabBar->baseColor(), 0));
            painter->drawLine(option->rect.topLeft(), option->rect.topRight());
            return;
        }
    }

    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QString ProxyStyle::name() const
{
    return baseStyle()->objectName();
}
