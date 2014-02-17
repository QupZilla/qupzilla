/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2014  David Rosca <nowrep@gmail.com>
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
#include "autoscroller.h"
#include "framescroller.h"
#include "webview.h"
#include "webpage.h"

#include <QMouseEvent>
#include <QWebFrame>
#include <QLabel>
#include <QDebug>

AutoScroller::AutoScroller(const QString &settings, QObject* parent)
    : QObject(parent)
    , m_view(0)
{
    Q_UNUSED(settings)

    m_indicator = new QLabel;
    m_indicator->resize(30, 30);
    m_indicator->setStyleSheet("QLabel{background-color: red;}");
    m_indicator->installEventFilter(this);

    m_frameScroller = new FrameScroller(this);
}

bool AutoScroller::mouseMove(QObject* obj, QMouseEvent* event)
{
    Q_UNUSED(obj)

    if (m_indicator->isVisible()) {
        QRect rect = indicatorGlobalRect();
        int xlength = 0;
        int ylength = 0;

        if (rect.left() > event->globalPos().x()) {
            xlength = event->globalPos().x() - rect.left();
        }
        else if (rect.right() < event->globalPos().x()) {
            xlength = event->globalPos().x() - rect.right();
        }
        if (rect.top() > event->globalPos().y()) {
            ylength = event->globalPos().y() - rect.top();
        }
        else if (rect.bottom() < event->globalPos().y()) {
            ylength = event->globalPos().y() - rect.bottom();
        }

        m_frameScroller->startScrolling(xlength, ylength);
    }

    return false;
}

bool AutoScroller::mousePress(QObject* obj, QMouseEvent* event)
{
    bool middleButton = event->buttons() == Qt::MiddleButton;
    WebView* view = qobject_cast<WebView*>(obj);
    Q_ASSERT(view);

    // Start?
    if (m_view != view && middleButton) {
        return showIndicator(view, event->pos());
    }
    else if (!m_indicator->isVisible() && middleButton) {
        return showIndicator(view, event->pos());
    }

    // Stop
    if (m_indicator->isVisible()) {
        stopScrolling();
        return true;
    }

    return false;
}

bool AutoScroller::mouseRelease(QObject* obj, QMouseEvent* event)
{
    Q_UNUSED(obj)

    if (m_indicator->isVisible()) {
        if (!indicatorGlobalRect().contains(event->globalPos())) {
            stopScrolling();
        }
        return true;
    }

    return false;
}

bool AutoScroller::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == m_indicator) {
        switch (event->type()) {
        case QEvent::Enter:
            m_frameScroller->stopScrolling();
            break;

        case QEvent::Wheel:
        case QEvent::Hide:
        case QEvent::MouseButtonPress:
            stopScrolling();
            break;

        default:
            break;
        }
    }

    return false;
}

bool AutoScroller::showIndicator(WebView* view, const QPoint &pos)
{
    QWebFrame* frame = view->page()->frameAt(pos);

    if (!frame) {
        return false;
    }

    const QWebHitTestResult res = frame->hitTestContent(pos);

    if (res.isContentEditable() || !res.linkUrl().isEmpty()) {
        return false;
    }

    bool vertical = frame->scrollBarGeometry(Qt::Vertical).isValid();
    bool horizontal = frame->scrollBarGeometry(Qt::Horizontal).isValid();

    if (!vertical && !horizontal) {
        return false;
    }

    m_view = view;

    QPoint p;
    p.setX(pos.x() - m_indicator->width() / 2);
    p.setY(pos.y() - m_indicator->height() / 2);

    m_indicator->setParent(view->parentWidget());
    m_indicator->move(p);
    m_indicator->show();

    m_frameScroller->setFrame(frame);

    m_view->grabMouse();
    return true;
}

void AutoScroller::stopScrolling()
{
    m_view->releaseMouse();

    m_indicator->hide();
    m_indicator->setParent(0);
    m_frameScroller->stopScrolling();
}

QRect AutoScroller::indicatorGlobalRect() const
{
    QPoint pos = m_indicator->parentWidget()->mapToGlobal(m_indicator->geometry().topLeft());
    return QRect(pos.x(), pos.y(), m_indicator->width(), m_indicator->height());
}
