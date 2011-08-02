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
#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <QWidget>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QResizeEvent>

class AnimatedWidget : public QWidget
{
    Q_OBJECT
public:
    enum Direction { Down, Up };
    explicit AnimatedWidget(const Direction &direction = Down, QWidget* parent = 0);
    ~AnimatedWidget();

    QWidget* widget() { return m_widget; }

public slots:
    void hide();
    void startAnimation();

private:
    void resizeEvent(QResizeEvent *e);

    QPropertyAnimation* m_positionAni;
    QPropertyAnimation* m_minHeightAni;
    QPropertyAnimation* m_maxHeightAni;
    QParallelAnimationGroup* m_aniGroup;

    QWidget* m_widget;

    int Y_SHOWN;
    int Y_HIDDEN;
    Direction m_direction;
};

#endif // NOTIFICATION_H
