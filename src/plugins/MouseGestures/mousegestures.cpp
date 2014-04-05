/* ============================================================
* Mouse Gestures plugin for QupZilla
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
#include "mousegestures.h"
#include "webpage.h"
#include "tabbedwebview.h"
#include "tabwidget.h"
#include "mainapplication.h"
#include "browserwindow.h"
#include "mousegesturessettingsdialog.h"

#include "QjtMouseGestureFilter.h"
#include "QjtMouseGesture.h"

#include <QMouseEvent>
#include <QWebFrame>
#include <QSettings>

MouseGestures::MouseGestures(const QString &settingsPath, QObject* parent) :
    QObject(parent)
    , m_filter(0)
    , m_button(Qt::MiddleButton)
    , m_settingsFile(settingsPath + "extensions.ini")
{
    loadSettings();
    initFilter();
}

void MouseGestures::initFilter() {
    m_filter = new QjtMouseGestureFilter(false, m_button, 20);

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

    QjtMouseGesture* upLeftGesture = new QjtMouseGesture(DirectionList() << Up << Left, m_filter);
    connect(upLeftGesture, SIGNAL(gestured()), this, SLOT(upLeftGestured()));

    QjtMouseGesture* upRightGesture = new QjtMouseGesture(DirectionList() << Up << Right, m_filter);
    connect(upRightGesture, SIGNAL(gestured()), this, SLOT(upRightGestured()));

    m_filter->addGesture(upGesture);
    m_filter->addGesture(downGesture);
    m_filter->addGesture(leftGesture);
    m_filter->addGesture(rightGesture);

    m_filter->addGesture(downRightGesture);
    m_filter->addGesture(downLeftGesture);
    m_filter->addGesture(upDownGesture);
    m_filter->addGesture(upLeftGesture);
    m_filter->addGesture(upRightGesture);
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
    if (!m_settings) {
        m_settings = new MouseGesturesSettingsDialog(this, parent);
    }

    m_settings.data()->show();
    m_settings.data()->raise();
}

void MouseGestures::unloadPlugin()
{
    delete m_settings.data();
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

    m_view.data()->openUrlInNewTab(QUrl(), Qz::NT_SelectedNewEmptyTab);
}

void MouseGestures::leftGestured()
{
    if (!m_view) {
        return;
    }

    if (QApplication::isRightToLeft()) {
        m_view.data()->forward();
    }
    else {
        m_view.data()->back();
    }
}

void MouseGestures::rightGestured()
{
    if (!m_view) {
        return;
    }

    if (QApplication::isRightToLeft()) {
        m_view.data()->back();
    }
    else {
        m_view.data()->forward();
    }
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

void MouseGestures::upLeftGestured()
{
    TabbedWebView* view = qobject_cast<TabbedWebView*>(m_view.data());
    if (!view) {
        return;
    }

    if (QApplication::isRightToLeft()) {
        view->tabWidget()->nextTab();
    }
    else {
        view->tabWidget()->previousTab();
    }
}

void MouseGestures::upRightGestured()
{
    TabbedWebView* view = qobject_cast<TabbedWebView*>(m_view.data());
    if (!view) {
        return;
    }

    if (QApplication::isRightToLeft()) {
        view->tabWidget()->previousTab();
    }
    else {
        view->tabWidget()->nextTab();
    }
}

void MouseGestures::setGestureButton(Qt::MouseButton button)
{
    if (m_filter) {
        m_filter->clearGestures(true);
        delete m_filter;
        m_filter = NULL;
    }

    m_button = button;
    initFilter();
}

void MouseGestures::setGestureButtonByIndex(int index)
{
    switch (index) {
	default:
        case 0:
	    m_button = Qt::MiddleButton;
	    break;
        case 1:
	    m_button = Qt::RightButton;
	    break;
        case 2:
	    m_button = Qt::LeftButton;
	    break;
    }

    setGestureButton(m_button);
}

Qt::MouseButton MouseGestures::gestureButton() const
{
    return m_button;
}

int MouseGestures::buttonToIndex() const
{
    switch (m_button) {
	default:
	case Qt::MiddleButton: return 0;
	case Qt::RightButton: return 1;
	case Qt::LeftButton: return 2;
    }
}

void MouseGestures::loadSettings()
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);

    settings.beginGroup("MouseGestures");
    setGestureButtonByIndex(settings.value("Button", 0).toInt());
    settings.endGroup();
}

void MouseGestures::saveSettings()
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);

    settings.beginGroup("MouseGestures");
    settings.setValue("Button", buttonToIndex());
    settings.endGroup();
}

MouseGestures::~MouseGestures()
{
    m_filter->clearGestures(true);
    delete m_filter;
}
