/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
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

#include "qz_namespace.h"

class QPropertyAnimation;
class QParallelAnimationGroup;

class QT_QUPZILLA_EXPORT AnimatedWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int fixedheight READ height WRITE setFixedHeight)

public:
    enum Direction { Down, Up };
    explicit AnimatedWidget(const Direction &direction = Down, int duration = 300, QWidget* parent = 0);
    ~AnimatedWidget();

    QWidget* widget() { return m_widget; }

public slots:
    void hide();
    void startAnimation();

private:
    void resizeEvent(QResizeEvent* e);

    QPropertyAnimation* m_positionAni;
    QPropertyAnimation* m_heightAni;
    QParallelAnimationGroup* m_aniGroup;

    QWidget* m_widget;

    int Y_SHOWN;
    int Y_HIDDEN;
    Direction m_direction;
};

#endif // NOTIFICATION_H
