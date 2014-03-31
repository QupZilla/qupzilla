/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2013-2014  David Rosca <nowrep@gmail.com>
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
#include "macwebviewscroller.h"

#include <QApplication>
#include <QWheelEvent>
#include <QWebView>
#include <QTimer>

// Workaround for QTBUG-22269 (Extremely slow scrolling on Apple trackpads)
// https://bugreports.qt-project.org/browse/QTBUG-22269

MacWebViewScroller::MacWebViewScroller(QWebView* view)
    : QObject(view)
    , m_view(view)
    , m_timerRunning(false)
    , m_delta(0)
{
    view->installEventFilter(this);
}

bool MacWebViewScroller::eventFilter(QObject* obj, QEvent* event)
{
    if (obj != m_view || event->type() != QEvent::Wheel) {
        return false;
    }

    QWheelEvent* ev = static_cast<QWheelEvent*>(event);
    if (ev->buttons() != Qt::NoButton || ev->modifiers() != Qt::NoModifier) {
        return false;
    }

    if (!m_timerRunning) {
        m_delta = ev->delta();
        m_pos = ev->pos();
        m_globalPos = ev->globalPos();

        QTimer::singleShot(25, this, SLOT(sendWheelEvent()));
        m_timerRunning = true;
    }
    else {
        m_delta += ev->delta();
    }

    return true;
}

void MacWebViewScroller::sendWheelEvent()
{
    QWheelEvent ev(m_pos, m_delta, Qt::NoButton, Qt::NoModifier);
    m_view->event(&ev);
    m_timerRunning = false;
}
