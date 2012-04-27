/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
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

#include <QKeyEvent>
#include <QApplication>
#include <QStyle>

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
        case Qt::Key_End:
        case Qt::Key_Home:
            if (keyEvent->modifiers() & Qt::ControlModifier) {
                return false;
            }
            break;

        case Qt::Key_Left:
        case Qt::Key_Right:
            close();
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

        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
            return false;
        } // switch (keyEvent->key())

        (static_cast<QObject*>(focusProxy()))->event(keyEvent);
        return true;
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
