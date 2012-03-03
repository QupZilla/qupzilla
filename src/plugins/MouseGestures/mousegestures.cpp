#include "mousegestures.h"
#include "QjtMouseGestureFilter.h"
#include "QjtMouseGesture.h"
#include "webview.h"
#include "mainapplication.h"
#include "qupzilla.h"
#include "mousegesturessettingsdialog.h"

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
    m_view->stop();
}

void MouseGestures::downGestured()
{
    m_view->openUrlInNewTab(QUrl(), Qz::NT_CleanSelectedTabAtTheEnd);
}

void MouseGestures::leftGestured()
{
    m_view->back();
}

void MouseGestures::rightGestured()
{
    m_view->forward();
}

void MouseGestures::downRightGestured()
{
    m_view->closeView();
}

void MouseGestures::downLeftGestured()
{
    m_view->load(mApp->getWindow()->homepageUrl());
}

void MouseGestures::upDownGestured()
{
    m_view->reload();
}

MouseGestures::~MouseGestures()
{
    m_filter->clearGestures(true);
    delete m_filter;
}
