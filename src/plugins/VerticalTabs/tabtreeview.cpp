/* ============================================================
* VerticalTabs plugin for QupZilla
* Copyright (C) 2018 David Rosca <nowrep@gmail.com>
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
#include "tabtreeview.h"
#include "tabtreedelegate.h"
#include "loadinganimator.h"

#include "tabmodel.h"
#include "webtab.h"
#include "tabcontextmenu.h"

#include <QToolTip>
#include <QHoverEvent>
#include <QApplication>

TabTreeView::TabTreeView(QWidget* parent)
    : QTreeView(parent)
{
    setDragEnabled(true);
    setAcceptDrops(true);
    setHeaderHidden(true);
    setUniformRowHeights(true);
    setDropIndicatorShown(true);
    setAllColumnsShowFocus(true);
    setMouseTracking(true);
    setIndentation(0);

    m_delegate = new TabTreeDelegate(this);
    setItemDelegate(m_delegate);
}

bool TabTreeView::areTabsInOrder() const
{
    return m_tabsInOrder;
}

void TabTreeView::setTabsInOrder(bool enable)
{
    m_tabsInOrder = enable;
}

void TabTreeView::drawBranches(QPainter *, const QRect &, const QModelIndex &) const
{
    // Disable drawing branches
}

void TabTreeView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    if (current.data(TabModel::CurrentTabRole).toBool()) {
        QTreeView::currentChanged(current, previous);
    } else if (previous.data(TabModel::CurrentTabRole).toBool()) {
        setCurrentIndex(previous);
    }
}

void TabTreeView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    QTreeView::dataChanged(topLeft, bottomRight, roles);

    if (roles.size() == 1 && roles.at(0) == TabModel::CurrentTabRole && topLeft.data(TabModel::CurrentTabRole).toBool()) {
        setCurrentIndex(topLeft);
    }
}

bool TabTreeView::viewportEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::MouseButtonPress: {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        const QModelIndex index = indexAt(me->pos());
        WebTab *tab = index.data(TabModel::WebTabRole).value<WebTab*>();
        if (me->buttons() == Qt::MiddleButton && tab) {
            tab->closeTab();
        }
        if (me->buttons() != Qt::LeftButton) {
            m_pressedIndex = QModelIndex();
            m_pressedButton = NoButton;
            break;
        }
        m_pressedIndex = index;
        m_pressedButton = buttonAt(me->pos(), m_pressedIndex);
        if (m_pressedIndex.isValid()) {
            if (m_pressedButton == ExpandButton) {
                if (isExpanded(m_pressedIndex)) {
                    collapse(m_pressedIndex);
                } else {
                    expand(m_pressedIndex);
                }
            } else if (m_pressedButton == NoButton && tab) {
                tab->makeCurrentTab();
            }
        }
        break;
    }

    case QEvent::MouseButtonRelease: {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        if (me->buttons() != Qt::NoButton) {
            break;
        }
        const QModelIndex index = indexAt(me->pos());
        if (m_pressedIndex != index) {
            break;
        }
        DelegateButton button = buttonAt(me->pos(), index);
        if (m_pressedButton == button) {
            if (m_pressedButton == ExpandButton) {
                me->accept();
                return true;
            }
            WebTab *tab = index.data(TabModel::WebTabRole).value<WebTab*>();
            if (tab) {
                if (m_pressedButton == CloseButton) {
                    tab->closeTab();
                } else if (m_pressedButton == AudioButton) {
                    tab->toggleMuted();
                }
            }
        }
        break;
    }

    case QEvent::ToolTip: {
        QHelpEvent *he = static_cast<QHelpEvent*>(event);
        const QModelIndex index = indexAt(he->pos());
        DelegateButton button = buttonAt(he->pos(), index);
        if (button == AudioButton) {
            const bool muted = index.data(TabModel::AudioMutedRole).toBool();
            QToolTip::showText(he->globalPos(), muted ? tr("Unmute Tab") : tr("Mute Tab"), this, visualRect(index));
            he->accept();
            return true;
        } else if (button == CloseButton) {
            QToolTip::showText(he->globalPos(), tr("Close Tab"), this, visualRect(index));
            he->accept();
            return true;
        }
        break;
    }

    case QEvent::ContextMenu: {
        QContextMenuEvent *ce = static_cast<QContextMenuEvent*>(event);
        const QModelIndex index = indexAt(ce->pos());
        WebTab *tab = index.data(TabModel::WebTabRole).value<WebTab*>();
        if (tab) {
            TabContextMenu menu(tab, Qt::Vertical, m_tabsInOrder);
            menu.exec(ce->globalPos());
        }
        break;
    }

    default:
        break;
    }
    return QTreeView::viewportEvent(event);
}

TabTreeView::DelegateButton TabTreeView::buttonAt(const QPoint &pos, const QModelIndex &index) const
{
    if (m_delegate->expandButtonRect(index).contains(pos)) {
        return ExpandButton;
    } else if (m_delegate->audioButtonRect(index).contains(pos)) {
        return AudioButton;
    } else if (m_delegate->closeButtonRect(index).contains(pos)) {
        return CloseButton;
    }
    return NoButton;
}
