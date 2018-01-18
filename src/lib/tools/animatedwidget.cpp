/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018  David Rosca <nowrep@gmail.com>
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
#include "animatedwidget.h"

#include <QResizeEvent>

AnimatedWidget::AnimatedWidget(const Direction &direction, int duration, QWidget* parent)
    : QWidget(parent)
    , m_direction(direction)
    , m_stepHeight(0)
    , m_stepY(0)
    , m_startY(0)
    , m_widget(new QWidget(this))
{
    m_timeLine.setDuration(duration);
    m_timeLine.setFrameRange(0, 100);
    connect(&m_timeLine, SIGNAL(frameChanged(int)), this, SLOT(animateFrame(int)));

    setMaximumHeight(0);
}

void AnimatedWidget::startAnimation()
{
    if (m_timeLine.state() == QTimeLine::Running) {
        return;
    }

    int shown = 0;
    int hidden = 0;

    if (m_direction == Down) {
        shown = 0;
        hidden = -m_widget->height();
    }

    m_widget->move(QPoint(m_widget->pos().x(), hidden));

    m_stepY = (hidden - shown) / 100.0;
    m_startY = hidden;
    m_stepHeight = m_widget->height() / 100.0;

    m_timeLine.setDirection(QTimeLine::Forward);
    m_timeLine.start();
}

void AnimatedWidget::animateFrame(int frame)
{
    setFixedHeight(frame * m_stepHeight);
    m_widget->move(pos().x(), m_startY - frame * m_stepY);
}

void AnimatedWidget::hide()
{
    if (m_timeLine.state() == QTimeLine::Running) {
        return;
    }

    m_timeLine.setDirection(QTimeLine::Backward);
    m_timeLine.start();

    connect(&m_timeLine, SIGNAL(finished()), this, SLOT(close()));

    QWidget* p = parentWidget();
    if (p) {
        p->setFocus();
    }
}

void AnimatedWidget::resizeEvent(QResizeEvent* event)
{
    if (event->size().width() != m_widget->width()) {
        m_widget->resize(event->size().width(), m_widget->height());
    }

    QWidget::resizeEvent(event);
}
