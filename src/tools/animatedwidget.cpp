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
#include "animatedwidget.h"
#include <QDebug>

AnimatedWidget::AnimatedWidget(const Direction &direction, QWidget* parent)
    : QWidget(parent)
    , m_widget(new QWidget(this))
    , m_direction(direction)
{
    m_positionAni = new QPropertyAnimation(m_widget, "pos");
    m_positionAni->setDuration(300);

    m_minHeightAni = new QPropertyAnimation(this, "minimumHeight");
    m_minHeightAni->setDuration(300);

    m_maxHeightAni = new QPropertyAnimation(this, "maximumHeight");
    m_maxHeightAni->setDuration(300);

    m_aniGroup = new QParallelAnimationGroup();
    m_aniGroup->addAnimation(m_positionAni);
    m_aniGroup->addAnimation(m_minHeightAni);
    m_aniGroup->addAnimation(m_maxHeightAni);

    setMaximumHeight(0);
}

void AnimatedWidget::startAnimation()
{
    if (m_direction == Down) {
        Y_SHOWN = 0;
        Y_HIDDEN = -m_widget->height();
    } else if (m_direction == Up) {
        Y_SHOWN = 0;
        Y_HIDDEN = 0;
    }

    m_widget->move(QPoint(m_widget->pos().x(), Y_HIDDEN));

    m_positionAni->setEndValue(QPoint(m_widget->pos().x(), Y_SHOWN));
    m_minHeightAni->setEndValue(m_widget->height());
    m_maxHeightAni->setEndValue(m_widget->height());

    m_aniGroup->start();
}

void AnimatedWidget::hide()
{
    m_positionAni->setEndValue(QPoint(m_widget->pos().x(), Y_HIDDEN));
    m_minHeightAni->setEndValue(0);
    m_maxHeightAni->setEndValue(0);

    m_aniGroup->start();
    connect(m_aniGroup, SIGNAL(finished()), this, SLOT(close()));
}

void AnimatedWidget::resizeEvent(QResizeEvent *event)
{
    if (event->size().width() != m_widget->width())
        m_widget->resize(event->size().width(), m_widget->height());

    QWidget::resizeEvent(event);
}

AnimatedWidget::~AnimatedWidget()
{
    delete m_positionAni;
    delete m_minHeightAni;
    delete m_maxHeightAni;
    delete m_aniGroup;

    delete m_widget;
}
