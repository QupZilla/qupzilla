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
#ifndef RSSNOTIFICATION_H
#define RSSNOTIFICATION_H

#include <QWidget>
#include <QTimeLine>

namespace Ui {
    class RSSNotification;
}

class RSSNotification : public QWidget
{
    Q_OBJECT

public:
    explicit RSSNotification(QString host, QWidget *parent = 0);
    ~RSSNotification();

private slots:
    void hide();
    void frameChanged(int frame);

private:
    Ui::RSSNotification *ui;
    QTimeLine* m_animation;
};

#endif // RSSNOTIFICATION_H
