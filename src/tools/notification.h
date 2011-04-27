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
#include <QTimeLine>
#include <QTimer>

class Notification : public QWidget
{
    Q_OBJECT
public:
    explicit Notification(QWidget* parent = 0);

public slots:
    void hide();
    void startAnimation();

private slots:
    void frameChanged(int frame);

private:
    QTimeLine* m_animation;
};

#endif // NOTIFICATION_H
