#include "notification.h"

Notification::Notification(QWidget *parent) :
    QWidget(parent)
   ,m_animation(0)
{
    setMinimumHeight(1);
    setMaximumHeight(1);
    setUpdatesEnabled(false);
}
void Notification::startAnimation()
{
    m_animation = new QTimeLine(300, this);
    m_animation->setFrameRange(0, sizeHint().height());
    setMinimumHeight(1);
    setMaximumHeight(1);
    setUpdatesEnabled(true);
    connect(m_animation, SIGNAL(frameChanged(int)),this, SLOT(frameChanged(int)));
    QTimer::singleShot(1, m_animation, SLOT(start()));
}

void Notification::hide()
{
    if (!m_animation) {
        close();
        return;
    }
    m_animation->setDirection(QTimeLine::Backward);

    m_animation->stop();
    m_animation->start();
    connect(m_animation, SIGNAL(finished()), this, SLOT(close()));
}

void Notification::frameChanged(int frame)
{
    setMinimumHeight(frame);
    setMaximumHeight(frame);
}
