/* ============================================================
* Mouse Gestures plugin for QupZilla
* Copyright (C) 2012  David Rosca <nowrep@gmail.com>
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
#include "mousegestures.h"
#include "webview.h"
#include "webpage.h"
#include "mainapplication.h"
#include "qupzilla.h"
#include "mousegesturessettingsdialog.h"

#include "QjtMouseGestureFilter.h"
#include "QjtMouseGesture.h"

#include <QMouseEvent>
#include <QWebFrame>

MouseGestures::MouseGestures(QObject* parent) :
    QObject(parent)
{
    m_filter = new QjtMouseGestureFilter(false, Qt::MiddleButton, 20);

    QjtMouseGesture* upGesture = new QjtMouseGesture(DirectionList() << Up, m_filter);
    connect(upGesture, SIGNAL(gestured()), this, SLOT(upGestured()));

    QjtMouseGesture* downGesture = new QjtMouseGesture(DirectionList() << Down, m_filter);
    connect(downGesture, SIGNAL(gestured()), this, SLOT(downGestured()));

    QjtMouseGesture* leftGesture = new QjtMouseGesture(DirectionList() << Left, m_filter);
    connect(leftGesture, SIGNAL(gestured()), this, SLOT(leftGestured()));

    QjtMouseGesture* rightGesture = new QjtMouseGesture(DirectionList() << Right, m_filter);
    connect(rightGesture, SIGNAL(gestured()), this, SLOT(rightGestured()));

    QjtMouseGesture* downRightGesture = new QjtMouseGesture(DirectionList() << Down << Right, m_filter);
    connect(downRightGesture, SIGNAL(gestured()), this, SLOT(downRightGestured()));

    QjtMouseGesture* downLeftGesture = new QjtMouseGesture(DirectionList() << Down << Left, m_filter);
    connect(downLeftGesture, SIGNAL(gestured()), this, SLOT(downLeftGestured()));

    QjtMouseGesture* upDownGesture = new QjtMouseGesture(DirectionList() << Up << Down, m_filter);
    connect(upDownGesture, SIGNAL(gestured()), this, SLOT(upDownGestured()));

    m_filter->addGesture(upGesture);
    m_filter->addGesture(downGesture);
    m_filter->addGesture(leftGesture);
    m_filter->addGesture(rightGesture);

    m_filter->addGesture(downRightGesture);
    m_filter->addGesture(downLeftGesture);
    m_filter->addGesture(upDownGesture);
}

bool MouseGestures::mousePress(QObject* obj, QMouseEvent* event)
{
    m_view = qobject_cast<WebView*>(obj);

    QWebFrame* frame = m_view.data()->page()->mainFrame();

    if (frame->scrollBarGeometry(Qt::Vertical).contains(event->pos()) ||
            frame->scrollBarGeometry(Qt::Horizontal).contains(event->pos())) {
        return false;
    }

    m_filter->mouseButtonPressEvent(event);

    return false;
}

bool MouseGestures::mouseRelease(QObject* obj, QMouseEvent* event)
{
    Q_UNUSED(obj)

    return m_filter->mouseButtonReleaseEvent(event);
}

bool MouseGestures::mouseMove(QObject* obj, QMouseEvent* event)
{
    Q_UNUSED(obj)

    m_filter->mouseMoveEvent(event);

    return false;
}

void MouseGestures::showSettings(QWidget* parent)
{
    MouseGesturesSettingsDialog* d = new MouseGesturesSettingsDialog(parent);
    d->show();
}

void MouseGestures::upGestured()
{
    if (!m_view) {
        return;
    }

    m_view.data()->stop();
}

void MouseGestures::downGestured()
{
    if (!m_view) {
        return;
    }

    m_view.data()->openUrlInNewTab(QUrl(), Qz::NT_CleanSelectedTabAtTheEnd);
}

void MouseGestures::leftGestured()
{
    if (!m_view) {
        return;
    }

    m_view.data()->back();
}

void MouseGestures::rightGestured()
{
    if (!m_view) {
        return;
    }

    m_view.data()->forward();
}

void MouseGestures::downRightGestured()
{
    if (!m_view) {
        return;
    }

    m_view.data()->closeView();
}

void MouseGestures::downLeftGestured()
{
    if (!m_view) {
        return;
    }

    m_view.data()->load(mApp->getWindow()->homepageUrl());
}

void MouseGestures::upDownGestured()
{
    if (!m_view) {
        return;
    }

    m_view.data()->reload();
}

MouseGestures::~MouseGestures()
{
    m_filter->clearGestures(true);
    delete m_filter;
}
