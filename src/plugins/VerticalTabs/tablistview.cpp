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
#include "tablistview.h"
#include "tablistdelegate.h"
#include "loadinganimator.h"

#include "tabmodel.h"
#include "webtab.h"
#include "tabcontextmenu.h"

#include <QTimer>
#include <QToolTip>
#include <QHoverEvent>

TabListView::TabListView(BrowserWindow *window, QWidget *parent)
    : QListView(parent)
    , m_window(window)
{
    setDragEnabled(true);
    setAcceptDrops(true);
    setUniformItemSizes(true);
    setDropIndicatorShown(true);
    setMouseTracking(true);
    setFlow(QListView::LeftToRight);
    setFocusPolicy(Qt::NoFocus);
    setFrameShape(QFrame::NoFrame);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    m_delegate = new TabListDelegate(this);
    setItemDelegate(m_delegate);

    updateHeight();
}

bool TabListView::isHidingWhenEmpty() const
{
    return m_hideWhenEmpty;
}

void TabListView::setHideWhenEmpty(bool enable)
{
    m_hideWhenEmpty = enable;
    updateVisibility();
}

void TabListView::updateIndex(const QModelIndex &index)
{
    QRect rect = visualRect(index);
    if (!rect.isValid()) {
        return;
    }
    // Need to update a little above/under to account for negative margins
    rect.moveTop(rect.y() - rect.height() / 2);
    rect.setHeight(rect.height() * 2);
    viewport()->update(rect);
}

void TabListView::adjustStyleOption(QStyleOptionViewItem *option)
{
    const QModelIndex index = option->index;

    option->state.setFlag(QStyle::State_Active, true);
    option->state.setFlag(QStyle::State_HasFocus, false);
    option->state.setFlag(QStyle::State_Selected, index.data(TabModel::CurrentTabRole).toBool());

    if (!index.isValid()) {
        option->viewItemPosition = QStyleOptionViewItem::Invalid;
    } else if (model()->rowCount() == 1) {
        option->viewItemPosition = QStyleOptionViewItem::OnlyOne;
    } else {
        if (!indexBefore(index).isValid()) {
            option->viewItemPosition = QStyleOptionViewItem::Beginning;
        } else if (!indexAfter(index).isValid()) {
            option->viewItemPosition = QStyleOptionViewItem::End;
        } else {
            option->viewItemPosition = QStyleOptionViewItem::Middle;
        }
    }
}

QModelIndex TabListView::indexAfter(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }
    const QRect rect = visualRect(index);
    return indexAt(QPoint(rect.right() + rect.width() / 2, rect.y()));
}

QModelIndex TabListView::indexBefore(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }
    const QRect rect = visualRect(index);
    return indexAt(QPoint(rect.left() - rect.width() / 2, rect.y()));
}

void TabListView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    if (current.data(TabModel::CurrentTabRole).toBool()) {
        QListView::currentChanged(current, previous);
    } else if (previous.data(TabModel::CurrentTabRole).toBool()) {
        setCurrentIndex(previous);
    }
}

void TabListView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    QListView::dataChanged(topLeft, bottomRight, roles);

    if (roles.size() == 1 && roles.at(0) == TabModel::CurrentTabRole && topLeft.data(TabModel::CurrentTabRole).toBool()) {
        setCurrentIndex(topLeft);
    }
}

void TabListView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    QListView::rowsInserted(parent, start, end);

    updateVisibility();
}

void TabListView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    QListView::rowsAboutToBeRemoved(parent, start, end);

    QTimer::singleShot(0, this, &TabListView::updateVisibility);
}

bool TabListView::viewportEvent(QEvent *event)
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
        if (m_pressedButton == NoButton && tab) {
            tab->makeCurrentTab();
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
            WebTab *tab = index.data(TabModel::WebTabRole).value<WebTab*>();
            if (tab && m_pressedButton == AudioButton) {
                tab->toggleMuted();
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
        } else if (button == NoButton) {
            QToolTip::showText(he->globalPos(), index.data().toString(), this, visualRect(index));
            he->accept();
            return true;
        }
        break;
    }

    case QEvent::ContextMenu: {
        QContextMenuEvent *ce = static_cast<QContextMenuEvent*>(event);
        const QModelIndex index = indexAt(ce->pos());
        WebTab *tab = index.data(TabModel::WebTabRole).value<WebTab*>();
        const int tabIndex = tab ? tab->tabIndex() : -1;
        TabContextMenu::Options options = TabContextMenu::HorizontalTabs | TabContextMenu::ShowDetachTabAction;
        TabContextMenu menu(tabIndex, m_window, options);
        menu.exec(ce->globalPos());
        break;
    }

    case QEvent::StyleChange:
        updateHeight();
        break;

    default:
        break;
    }
    return QListView::viewportEvent(event);
}

TabListView::DelegateButton TabListView::buttonAt(const QPoint &pos, const QModelIndex &index) const
{
    if (m_delegate->audioButtonRect(index).contains(pos)) {
        return AudioButton;
    }
    return NoButton;
}

void TabListView::updateVisibility()
{
    setVisible(!m_hideWhenEmpty || model()->rowCount() > 0);
}

void TabListView::updateHeight()
{
    setFixedHeight(m_delegate->sizeHint(viewOptions(), QModelIndex()).height());
}
