/* ============================================================
* AutoScroll - Autoscroll for QupZilla
* Copyright (C) 2014-2017 David Rosca <nowrep@gmail.com>
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
#include "webhittestresult.h"

#include <QApplication>
#include <QMouseEvent>
#include <QSettings>
#include <QLabel>
#include <QIcon>

ScrollIndicator::ScrollIndicator(QWidget *parent)
    : QLabel(parent)
{
    resize(33, 33);
    setContentsMargins(0, 0, 0, 0);
}

Qt::Orientations ScrollIndicator::orientations() const
{
    return m_orientations;
}

void ScrollIndicator::setOrientations(Qt::Orientations orientations)
{
    m_orientations = orientations;

    if (m_orientations == Qt::Vertical) {
        setPixmap(QIcon(QSL(":/autoscroll/data/scroll_vertical.png")).pixmap(33));
    } else if (m_orientations == Qt::Horizontal) {
        setPixmap(QIcon(QSL(":/autoscroll/data/scroll_horizontal.png")).pixmap(33));
    } else {
        setPixmap(QIcon(QSL(":/autoscroll/data/scroll_all.png")).pixmap(33));
    }

    update();
}

void ScrollIndicator::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QRectF r(rect());
    r.adjust(1, 1, -1, -1);

    QColor c1(Qt::gray);
    c1.setAlpha(190);

    QColor c2(Qt::white);
    c2.setAlpha(190);

    QRadialGradient g(r.center(), r.height() / 2.0);
    g.setColorAt(1, c1);
    g.setColorAt(0.7, c2);

    p.setPen(Qt::NoPen);
    p.setBrush(g);
    p.drawEllipse(r);

    QLabel::paintEvent(event);
}

AutoScroller::AutoScroller(const QString &settingsFile, QObject* parent)
    : QObject(parent)
    , m_view(0)
    , m_settingsFile(settingsFile)
{
    m_indicator = new ScrollIndicator;
    m_indicator->installEventFilter(this);

    QSettings settings(m_settingsFile, QSettings::IniFormat);
    settings.beginGroup("AutoScroll");

    m_frameScroller = new FrameScroller(this);
    m_frameScroller->setScrollDivider(settings.value("ScrollDivider", 8.0).toDouble());

    settings.endGroup();
}

AutoScroller::~AutoScroller()
{
    delete m_indicator;
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

bool AutoScroller::wheel(QObject *obj, QWheelEvent *event)
{
    Q_UNUSED(obj)
    Q_UNUSED(event);

    if (m_indicator->isVisible()) {
        stopScrolling();
        return true;
    }

    return false;
}

double AutoScroller::scrollDivider() const
{
    return m_frameScroller->scrollDivider();
}

void AutoScroller::setScrollDivider(double divider)
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);
    settings.beginGroup("AutoScroll");
    settings.setValue("ScrollDivider", divider);
    settings.endGroup();

    m_frameScroller->setScrollDivider(divider);
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
    const WebHitTestResult res = view->page()->hitTestContent(pos);

    if (res.isContentEditable() || !res.linkUrl().isEmpty() || res.tagName().endsWith(QL1S("frame"))) {
        return false;
    }

    QString source = QL1S("var out = {"
                          " vertical: window.innerWidth > document.documentElement.clientWidth,"
                          " horizontal: window.innerHeight > document.documentElement.clientHeight"
                          "};"
                          "out;");

    const QVariantMap &map = view->page()->execJavaScript(source, WebPage::SafeJsWorld).toMap();

    bool vertical = map.value(QSL("vertical")).toBool();
    bool horizontal = map.value(QSL("horizontal")).toBool();

    if (!vertical && !horizontal) {
        return false;
    }

    Qt::Orientations orientations;
    if (vertical) {
        orientations |= Qt::Vertical;
    }
    if (horizontal) {
        orientations |= Qt::Horizontal;
    }
    m_indicator->setOrientations(orientations);

    m_view = view;

    QPoint p;
    p.setX(pos.x() - m_indicator->width() / 2);
    p.setY(pos.y() - m_indicator->height() / 2);

    m_indicator->setParent(m_view->overlayWidget());
    m_indicator->move(m_view->mapTo(m_view->overlayWidget(), p));
    m_indicator->show();

    m_frameScroller->setPage(view->page());

    m_view->inputWidget()->grabMouse();
    QApplication::setOverrideCursor(Qt::ArrowCursor);

    return true;
}

void AutoScroller::stopScrolling()
{
    m_view->inputWidget()->releaseMouse();
    QApplication::restoreOverrideCursor();

    m_indicator->hide();
    m_indicator->setParent(0);
    m_frameScroller->stopScrolling();
}

QRect AutoScroller::indicatorGlobalRect() const
{
    QPoint pos = m_indicator->parentWidget()->mapToGlobal(m_indicator->geometry().topLeft());
    return QRect(pos.x(), pos.y(), m_indicator->width(), m_indicator->height());
}
