/*
 * This file is part of the mouse gesture package.
 * Copyright (C) 2006 Johan Thelin <e8johan@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the
 * following conditions are met:
 *
 *   - Redistributions of source code must retain the above
 *     copyright notice, this list of conditions and the
 *     following disclaimer.
 *   - Redistributions in binary form must reproduce the
 *     above copyright notice, this list of conditions and
 *     the following disclaimer in the documentation and/or
 *     other materials provided with the distribution.
 *   - The names of its contributors may be used to endorse
 *     or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "QjtMouseGestureFilter.h"
#include "QjtMouseGesture.h"
#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include "mousegesturerecognizer.h"

/*
 *  Internal support class to bridge the
 *  toolkit independent mouse gesture
 *  recognizer and the Qt specific
 *  implementation.
 */
class GestureCallbackToSignal : public Gesture::MouseGestureCallback
{
public:
    GestureCallbackToSignal(QjtMouseGesture* object) {
        m_object = object;
    }

    void callback() {
        m_object->emitGestured();
    }

private:
    QjtMouseGesture* m_object;
};

typedef QList<QjtMouseGesture*> GestureList;
typedef QList<GestureCallbackToSignal> BridgeList;

/*
 *  Private members of the QjtMouseGestureFilter class
 */
class QjtMouseGestureFilter::Private
{
public:
    Qt::MouseButton gestureButton;
    bool tracing;

    Gesture::MouseGestureRecognizer* mgr;
    QPixmap px;
    GestureList gestures;
    BridgeList bridges;
};

/**********************************************************/

QjtMouseGestureFilter::QjtMouseGestureFilter(bool allowDiagonals, Qt::MouseButton gestureButton, int minimumMovement, double minimumMatch, QObject* parent) : QObject(parent)
{
    d = new Private;

    d->gestureButton = gestureButton;
    d->tracing = false;

    d->mgr = new Gesture::MouseGestureRecognizer(minimumMovement, minimumMatch, allowDiagonals);
}

QjtMouseGestureFilter::~QjtMouseGestureFilter()
{
    delete d->mgr;
    delete d;
}

/*
 *  Converts the DirectionList to a Gesture::DirecionList,
 *  creates a bridge and adds the gesture to the recognizer.
 */
void QjtMouseGestureFilter::addGesture(QjtMouseGesture* gesture)
{
    Gesture::DirectionList dl;

    for (DirectionList::const_iterator source = gesture->directions().constBegin(); source != gesture->directions().end(); ++source) {
        dl.push_back(*source);
    }

    d->bridges.append(GestureCallbackToSignal(gesture));
    d->gestures.append(gesture);

    d->mgr->addGestureDefinition(Gesture::GestureDefinition(dl, &(d->bridges[ d->bridges.size() - 1 ])));
}

void QjtMouseGestureFilter::clearGestures(bool deleteGestures)
{
    if (deleteGestures)
        for (GestureList::const_iterator i = d->gestures.constBegin(); i != d->gestures.constEnd(); ++i) {
            delete *i;
        }

    d->gestures.clear();
    d->bridges.clear();
    d->mgr->clearGestureDefinitions();
}

bool QjtMouseGestureFilter::eventFilter(QObject* obj, QEvent* event)
{
    switch (event->type()) {
    case QEvent::MouseButtonPress:
        if (mouseButtonPressEvent(dynamic_cast<QMouseEvent*>(event), obj)) {
            return true;
        }

        break;

    case QEvent::MouseButtonRelease:
        if (mouseButtonReleaseEvent(dynamic_cast<QMouseEvent*>(event), obj)) {
            return true;
        }

        break;

    case QEvent::MouseMove:
        if (mouseMoveEvent(dynamic_cast<QMouseEvent*>(event), obj)) {
            return true;
        }

        break;

    case QEvent::Paint:
        if (paintEvent(obj, dynamic_cast<QPaintEvent*>(event))) {
            return true;
        }

    default:
        break;
    }

    return QObject::eventFilter(obj, event);
}

bool QjtMouseGestureFilter::mouseButtonPressEvent(QMouseEvent* event, QObject* obj)
{
    Q_UNUSED(obj)

    if (event->button() == d->gestureButton) {
//      d->px = QPixmap::grabWidget(static_cast<QWidget*>(obj));
        d->mgr->startGesture(event->pos().x(), event->pos().y());
        d->tracing = true;
    }

    return false;
}

bool QjtMouseGestureFilter::mouseButtonReleaseEvent(QMouseEvent* event, QObject* obj)
{
    Q_UNUSED(obj)

    if (d->tracing && event->button() == d->gestureButton) {
        d->tracing = false;
        return d->mgr->endGesture(event->pos().x(), event->pos().y());
//        d->px = QPixmap();
//  static_cast<QWidget*>(obj)->update();
    }

    return false;
}

bool QjtMouseGestureFilter::mouseMoveEvent(QMouseEvent* event, QObject* obj)
{
    Q_UNUSED(obj)

    if (d->tracing) {
        d->mgr->addPoint(event->pos().x(), event->pos().y());
//  static_cast<QWidget*>(obj)->update();
    }

    return false;
}

bool QjtMouseGestureFilter::paintEvent(QObject* obj, QPaintEvent* event)
{
    Q_UNUSED(event)
    if (d->tracing) {
        QWidget* wid = static_cast<QWidget*>(obj);
        QPainter painter(wid);
        painter.drawPixmap(0, 0, d->px);
        const Gesture::PosList points = d->mgr->currentPath();
        painter.save();
        QPen pe;
        pe.setColor(Qt::red);
        pe.setWidth(2);
        painter.setPen(pe);
        QVector<QPoint> pointPairs;
        for (Gesture::PosList::const_iterator iter = points.begin(); iter != points.end(); ++iter) {
            pointPairs << QPoint(iter->x, iter->y);
        }
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.drawPolyline(&pointPairs[0], pointPairs.count());
        painter.restore();
        painter.end();
        return true;
    }
    else {
        return false;
    }
}
