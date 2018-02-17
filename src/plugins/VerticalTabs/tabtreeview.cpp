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

#include <QTimer>
#include <QToolTip>
#include <QHoverEvent>

TabTreeView::TabTreeView(BrowserWindow *window, QWidget *parent)
    : QTreeView(parent)
    , m_window(window)
    , m_expandedSessionKey(QSL("VerticalTabs-expanded"))
{
    setDragEnabled(true);
    setAcceptDrops(true);
    setHeaderHidden(true);
    setUniformRowHeights(true);
    setDropIndicatorShown(true);
    setAllColumnsShowFocus(true);
    setMouseTracking(true);
    setFocusPolicy(Qt::NoFocus);
    setFrameShape(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setIndentation(0);

    m_delegate = new TabTreeDelegate(this);
    setItemDelegate(m_delegate);

    // Move scrollbar to the left
    setLayoutDirection(isRightToLeft() ? Qt::LeftToRight : Qt::RightToLeft);

    // Enable hover to force redrawing close button
    viewport()->setAttribute(Qt::WA_Hover);

    auto saveExpandedState = [this](const QModelIndex &index, bool expanded) {
        if (m_initializing) {
            return;
        }
        WebTab *tab = index.data(TabModel::WebTabRole).value<WebTab*>();
        if (tab) {
            tab->setSessionData(m_expandedSessionKey, expanded);
        }
    };
    connect(this, &TabTreeView::expanded, this, std::bind(saveExpandedState, std::placeholders::_1, true));
    connect(this, &TabTreeView::collapsed, this, std::bind(saveExpandedState, std::placeholders::_1, false));
}

int TabTreeView::backgroundIndentation() const
{
    return m_backgroundIndentation;
}

void TabTreeView::setBackgroundIndentation(int indentation)
{
    m_backgroundIndentation = indentation;
}

bool TabTreeView::areTabsInOrder() const
{
    return m_tabsInOrder;
}

void TabTreeView::setTabsInOrder(bool enable)
{
    m_tabsInOrder = enable;
}

bool TabTreeView::haveTreeModel() const
{
    return m_haveTreeModel;
}

void TabTreeView::setHaveTreeModel(bool enable)
{
    m_haveTreeModel = enable;
}

void TabTreeView::setModel(QAbstractItemModel *model)
{
    QTreeView::setModel(model);

    m_initializing = true;
    QTimer::singleShot(0, this, &TabTreeView::initView);
}

void TabTreeView::updateIndex(const QModelIndex &index)
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

void TabTreeView::adjustStyleOption(QStyleOptionViewItem *option)
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
        if (!indexAbove(index).isValid()) {
            option->viewItemPosition = QStyleOptionViewItem::Beginning;
        } else if (!indexBelow(index).isValid()) {
            option->viewItemPosition = QStyleOptionViewItem::End;
        } else {
            option->viewItemPosition = QStyleOptionViewItem::Middle;
        }
    }
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

void TabTreeView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    QTreeView::rowsInserted(parent, start, end);

    if (m_initializing) {
        return;
    }

    // Parent for WebTab is set after insertTab is emitted
    const QPersistentModelIndex index = model()->index(start, 0, parent);
    QTimer::singleShot(0, this, [=]() {
        if (!index.isValid()) {
            return;
        }
        QModelIndex idx = index;
        QVector<QModelIndex> stack;
        do {
            stack.append(idx);
            idx = idx.parent();
        } while (idx.isValid());
        for (const QModelIndex &index : qAsConst(stack)) {
            expand(index);
        }
        if (index.data(TabModel::CurrentTabRole).toBool()) {
            setCurrentIndex(index);
        }
    });
}

bool TabTreeView::viewportEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::MouseButtonPress: {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        const QModelIndex index = indexAt(me->pos());
        updateIndex(index);
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
        if (m_pressedButton == CloseButton) {
            me->accept();
            return true;
        }
        break;
    }

    case QEvent::MouseMove: {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        if (m_pressedButton == CloseButton) {
            me->accept();
            return true;
        }
        break;
    }

    case QEvent::MouseButtonRelease: {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        if (me->buttons() != Qt::NoButton) {
            break;
        }
        const QModelIndex index = indexAt(me->pos());
        updateIndex(index);
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
        if (m_pressedButton == CloseButton) {
            me->accept();
            return true;
        }
        break;
    }

    case QEvent::HoverEnter:
    case QEvent::HoverLeave:
    case QEvent::HoverMove: {
        QHoverEvent *he = static_cast<QHoverEvent*>(event);
        updateIndex(m_hoveredIndex);
        m_hoveredIndex = indexAt(he->pos());
        updateIndex(m_hoveredIndex);
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
        TabContextMenu::Options options = TabContextMenu::VerticalTabs | TabContextMenu::ShowDetachTabAction;
        if (m_tabsInOrder) {
            options |= TabContextMenu::ShowCloseOtherTabsActions;
        }
        TabContextMenu menu(tabIndex, m_window, options);
        addMenuActions(&menu, index);
        menu.exec(ce->globalPos());
        break;
    }

    default:
        break;
    }
    return QTreeView::viewportEvent(event);
}

void TabTreeView::initView()
{
    // Restore expanded state
    expandAll();
    QModelIndex index = model()->index(0, 0);
    while (index.isValid()) {
        WebTab *tab = index.data(TabModel::WebTabRole).value<WebTab*>();
        if (tab) {
            setExpanded(index, tab->sessionData().value(m_expandedSessionKey, true).toBool());
        }
        index = indexBelow(index);
    }

    m_initializing = false;
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

void TabTreeView::addMenuActions(QMenu *menu, const QModelIndex &index) const
{
    if (!m_haveTreeModel) {
        return;
    }

    menu->addSeparator();
    QMenu *m = menu->addMenu(tr("Tab Tree"));

    if (index.isValid() && model()->rowCount(index) > 0) {
        QPersistentModelIndex pindex = index;
        m->addAction(tr("Close Tree"), this, [=]() {
            QVector<WebTab*> tabs;
            reverseTraverse(pindex, [&](const QModelIndex &index) {
                WebTab *tab = index.data(TabModel::WebTabRole).value<WebTab*>();
                if (tab) {
                    tabs.append(tab);
                }
            });
            for (WebTab *tab : qAsConst(tabs)) {
                tab->closeTab();
            }
        });
    }

    m->addSeparator();
    m->addAction(tr("Expand All"), this, &TabTreeView::expandAll);
    m->addAction(tr("Collapse All"), this, &TabTreeView::collapseAll);
}

void TabTreeView::reverseTraverse(const QModelIndex &root, std::function<void(const QModelIndex&)> callback) const
{
    if (!root.isValid()) {
        return;
    }
    for (int i = 0; i < model()->rowCount(root); ++i) {
        reverseTraverse(model()->index(i, 0, root), callback);
    }
    callback(root);
}
