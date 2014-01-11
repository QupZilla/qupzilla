/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
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
#include "locationcompleterview.h"
#include "locationcompletermodel.h"
#include "locationcompleterdelegate.h"
#include "mainapplication.h"
#include "qupzilla.h"
#include "history.h"
#include "tabwidget.h"
#include "qzsettings.h"
#include "tabbedwebview.h"

#include <QKeyEvent>
#include <QApplication>
#include <QStyle>
#include <QScrollBar>

LocationCompleterView::LocationCompleterView()
    : QListView(0)
    , m_ignoreNextMouseMove(false)
{
    setWindowFlags(Qt::Popup);

    setUniformItemSizes(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);

    setMouseTracking(true);
    installEventFilter(this);
}

QPersistentModelIndex LocationCompleterView::hoveredIndex() const
{
    return m_hoveredIndex;
}

bool LocationCompleterView::eventFilter(QObject* object, QEvent* event)
{
    Q_UNUSED(object)
    // Event filter based on QCompleter::eventFilter from qcompleter.cpp

    switch (event->type()) {
    case QEvent::KeyPress: {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        QModelIndex curIndex = m_hoveredIndex;

        if ((keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down)
                && currentIndex() != curIndex) {
            setCurrentIndex(curIndex);
        }

        switch (keyEvent->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            if (qzSettings->showSwitchTab && !(keyEvent->modifiers() & Qt::ShiftModifier)) {
                QModelIndex idx = selectionModel()->currentIndex();
                if (idx.isValid()) {
                    TabPosition pos = idx.data(LocationCompleterModel::TabPositionRole).value<TabPosition>();
                    if (pos.windowIndex != -1) {
                        activateTab(pos);
                        return true;
                    }
                }
            }
            break;

        case Qt::Key_End:
        case Qt::Key_Home:
            if (keyEvent->modifiers() & Qt::ControlModifier) {
                return false;
            }
            break;

        case Qt::Key_Escape:
            close();
            return false;

        case Qt::Key_F4:
            if (keyEvent->modifiers() == Qt::AltModifier)  {
                close();
                return false;
            }
            break;

        case Qt::Key_Tab:
        case Qt::Key_Backtab: {
            Qt::Key k = keyEvent->key() == Qt::Key_Tab ? Qt::Key_Down : Qt::Key_Up;
            QKeyEvent ev(QKeyEvent::KeyPress, k, Qt::NoModifier);
            QApplication::sendEvent(this, &ev);
            return false;
        }

        case Qt::Key_Up:
            if (!curIndex.isValid()) {
                int rowCount = model()->rowCount();
                QModelIndex lastIndex = model()->index(rowCount - 1, 0);
                setCurrentIndex(lastIndex);
                return true;
            }
            else if (curIndex.row() == 0) {
                setCurrentIndex(QModelIndex());
                return true;
            }
            return false;

        case Qt::Key_Down:
            if (!curIndex.isValid()) {
                QModelIndex firstIndex = model()->index(0, 0);
                setCurrentIndex(firstIndex);
                return true;
            }
            else if (curIndex.row() == model()->rowCount() - 1) {
                setCurrentIndex(QModelIndex());
                scrollToTop();
                return true;
            }
            return false;

        case Qt::Key_Delete:
            if (viewport()->rect().contains(visualRect(curIndex)) &&
                    !curIndex.data(LocationCompleterModel::BookmarkRole).toBool()) {
                int id = curIndex.data(LocationCompleterModel::IdRole).toInt();
                model()->removeRow(curIndex.row(), curIndex.parent());

                mApp->history()->deleteHistoryEntry(id);
                return true;
            }
            break;

        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
            return false;

        case Qt::Key_Shift:
            // don't switch if there is no hovered or selected index to not disturb typing
            if (qzSettings->showSwitchTab && (selectionModel()->currentIndex().isValid() || m_hoveredIndex.isValid())) {
                static_cast<LocationCompleterDelegate*>(itemDelegate())->drawSwitchToTab(false);
                viewport()->update();
                return true;
            }
            break;
        } // switch (keyEvent->key())

        (static_cast<QObject*>(focusProxy()))->event(keyEvent);
        return true;
    }

    case QEvent::KeyRelease: {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

        switch (keyEvent->key()) {
        case Qt::Key_Shift:
            if (qzSettings->showSwitchTab) {
                static_cast<LocationCompleterDelegate*>(itemDelegate())->drawSwitchToTab(true);
                viewport()->update();
                return true;
            }
        }
    }


    case QEvent::Show:
        m_ignoreNextMouseMove = true;
        break;

    case QEvent::MouseButtonPress:
        if (!underMouse()) {
            close();
            return true;
        }
        break;

    case QEvent::ShortcutOverride:
    case QEvent::InputMethod:
        QApplication::sendEvent(focusProxy(), event);
        break;

    default:
        break;
    } // switch (event->type())

    return false;
}

void LocationCompleterView::close()
{
    emit closed();
    m_hoveredIndex = QPersistentModelIndex();

    QListView::hide();
    verticalScrollBar()->setValue(0);
    if (qzSettings->showSwitchTab) {
        static_cast<LocationCompleterDelegate*>(itemDelegate())->drawSwitchToTab(true);
    }
}

void LocationCompleterView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    m_hoveredIndex = current;

    QListView::currentChanged(current, previous);

    viewport()->update();
}

void LocationCompleterView::mouseMoveEvent(QMouseEvent* event)
{
    if (m_ignoreNextMouseMove || !isVisible()) {
        m_ignoreNextMouseMove = false;

        QListView::mouseMoveEvent(event);
        return;
    }

    QModelIndex last = m_hoveredIndex;
    QModelIndex atCursor = indexAt(mapFromGlobal(QCursor::pos()));

    if (atCursor.isValid()) {
        m_hoveredIndex = atCursor;
    }

    if (last != atCursor) {
        viewport()->update();
    }

    QListView::mouseMoveEvent(event);
}

void LocationCompleterView::mouseReleaseEvent(QMouseEvent* event)
{
    if (qzSettings->showSwitchTab && !(event->modifiers() & Qt::ShiftModifier) && m_hoveredIndex.isValid()) {
        TabPosition pos = m_hoveredIndex.data(LocationCompleterModel::TabPositionRole).value<TabPosition>();
        if (pos.windowIndex != -1) {
            event->accept();
            activateTab(pos);
        }
        else {
            QListView::mouseReleaseEvent(event);
        }
    }
    else {
        QListView::mouseReleaseEvent(event);
    }
}

void LocationCompleterView::activateTab(TabPosition pos)
{
    QupZilla* win = mApp->mainWindows().at(pos.windowIndex);
    if (mApp->getWindow() != win || mApp->getWindow()->tabWidget()->currentIndex() != pos.tabIndex) {
        emit aboutToActivateTab(pos);
        close();
        win->tabWidget()->setCurrentIndex(pos.tabIndex);
        win->show();
        win->activateWindow();
        win->raise();
    }
    else {
        close();
        win->weView()->setFocus();
    }
}
