#include "autosaver.h"
#include "mainapplication.h"

AutoSaver::AutoSaver(QObject *parent) :
     QObject(parent)
     ,p_mainApp(MainApplication::getInstance())
{
    m_timer.start(1000*5, this);
}

void AutoSaver::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timer.timerId() && p_mainApp->isChanged())
        emit saveApp();
    else
        QObject::timerEvent(event);
}
