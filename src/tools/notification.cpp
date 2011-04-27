/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  nowrep
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
#include "notification.h"

Notification::Notification(QWidget* parent) :
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
