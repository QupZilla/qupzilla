/* ============================================================
* Mouse Gestures plugin for QupZilla
* Copyright (C) 2013-2016 David Rosca <nowrep@gmail.com>
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
#include "locationbar.h"
#include "mousegesturessettingsdialog.h"

#include "QjtMouseGestureFilter.h"
#include "QjtMouseGesture.h"

#include <QMouseEvent>
#include <QWebEngineHistory>
#include <QSettings>

MouseGestures::MouseGestures(const QString &settingsPath, QObject* parent)
    : QObject(parent)
    , m_filter(0)
    , m_settingsFile(settingsPath + QL1S("/extensions.ini"))
    , m_button(Qt::MiddleButton)
    , m_enableRockerNavigation(false)
    , m_blockNextRightMouseRelease(false)
    , m_blockNextLeftMouseRelease(false)
{
    loadSettings();
}

void MouseGestures::initFilter()
{
    if (m_filter) {
        m_filter->clearGestures(true);
        delete m_filter;
    }

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

    QjtMouseGesture* downUpGesture = new QjtMouseGesture(DirectionList() << Down << Up, m_filter);
    connect(downUpGesture, SIGNAL(gestured()), this, SLOT(downUpGestured()));

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
    m_filter->addGesture(downUpGesture);
    m_filter->addGesture(upDownGesture);
    m_filter->addGesture(upLeftGesture);
    m_filter->addGesture(upRightGesture);
}

bool MouseGestures::mousePress(QObject* obj, QMouseEvent* event)
{
    m_view = qobject_cast<WebView*>(obj);

    if (m_enableRockerNavigation && event->buttons() == (Qt::RightButton | Qt::LeftButton)) {
        bool accepted = false;

        if (event->button() == Qt::LeftButton && m_view.data()->history()->canGoBack()) {
            m_view.data()->back();
            accepted = true;
        }
        else if (event->button() == Qt::RightButton && m_view.data()->history()->canGoForward()) {
            m_view.data()->forward();
            accepted = true;
        }

        if (accepted) {
            m_blockNextLeftMouseRelease = true;
            m_blockNextRightMouseRelease = true;
            return true;
        }
    }

    m_filter->mouseButtonPressEvent(event);

    return false;
}

bool MouseGestures::mouseRelease(QObject* obj, QMouseEvent* event)
{
    Q_UNUSED(obj)

    if (m_blockNextRightMouseRelease && event->button() == Qt::RightButton) {
        m_blockNextRightMouseRelease = false;
        return true;
    }

    if (m_blockNextLeftMouseRelease && event->button() == Qt::LeftButton) {
        m_blockNextLeftMouseRelease = false;
        return true;
    }

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
    TabbedWebView* view = qobject_cast<TabbedWebView*>(m_view.data());
    if (!view)
        return;

    BrowserWindow* window = view->browserWindow();
    if (!window)
        return;

    TabWidget* tabWidget = window->tabWidget();
    tabWidget->addView(QUrl(), Qz::NT_SelectedNewEmptyTab, true);
    tabWidget->setCurrentTabFresh(true);

    if (window->isFullScreen())
        window->showNavigationWithFullScreen();
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
    TabbedWebView *view = qobject_cast<TabbedWebView*>(m_view.data());
    if (!view)
        return;

    BrowserWindow *window = view->browserWindow();
    if (!window)
        return;

    TabWidget *tabWidget = window->tabWidget();
    if (!m_view) {
        return;
    }

    tabWidget->requestCloseTab(view->tabIndex());
}

void MouseGestures::downLeftGestured()
{
    if (!m_view) {
        return;
    }

    m_view.data()->load(mApp->getWindow()->homepageUrl());
}

void MouseGestures::downUpGestured()
{
    TabbedWebView* view = qobject_cast<TabbedWebView*>(m_view.data());
    if (!view)
        return;

    BrowserWindow* window = view->browserWindow();
    if (!window)
        return;

    TabWidget* tabWidget = window->tabWidget();
    tabWidget->duplicateTab(tabWidget->currentIndex());
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
    if (!view)
        return;

    BrowserWindow* window = view->browserWindow();
    if (!window)
        return;

    if (QApplication::isRightToLeft())
        window->tabWidget()->nextTab();
    else
        window->tabWidget()->previousTab();
}

void MouseGestures::upRightGestured()
{
    TabbedWebView* view = qobject_cast<TabbedWebView*>(m_view.data());
    if (!view)
        return;

    BrowserWindow* window = view->browserWindow();
    if (!window)
        return;

    if (QApplication::isRightToLeft())
        window->tabWidget()->previousTab();
    else
        window->tabWidget()->nextTab();
}

void MouseGestures::init()
{
    initFilter();

    // We need to override right mouse button events
    WebView::setForceContextMenuOnMouseRelease(m_button == Qt::RightButton || m_enableRockerNavigation);
}

void MouseGestures::setGestureButton(Qt::MouseButton button)
{
    m_button = button;
    init();
}

void MouseGestures::setGestureButtonByIndex(int index)
{
    switch (index) {
    case 0:
        m_button = Qt::MiddleButton;
        break;

    case 1:
        m_button = Qt::RightButton;
        break;

    default:
        m_button = Qt::NoButton;
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
    case Qt::MiddleButton:
        return 0;

    case Qt::RightButton:
        return 1;

    default:
        return 2;
    }
}

bool MouseGestures::rockerNavigationEnabled() const
{
    return m_enableRockerNavigation;
}

void MouseGestures::setRockerNavigationEnabled(bool enable)
{
    m_enableRockerNavigation = enable;
    init();
}

void MouseGestures::loadSettings()
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);

    settings.beginGroup("MouseGestures");
    setGestureButtonByIndex(settings.value("Button", 0).toInt());
    m_enableRockerNavigation = settings.value("RockerNavigation", true).toBool();
    settings.endGroup();

    init();
}

void MouseGestures::saveSettings()
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);

    settings.beginGroup("MouseGestures");
    settings.setValue("Button", buttonToIndex());
    settings.setValue("RockerNavigation", m_enableRockerNavigation);
    settings.endGroup();
}

MouseGestures::~MouseGestures()
{
    m_filter->clearGestures(true);
    delete m_filter;
}
