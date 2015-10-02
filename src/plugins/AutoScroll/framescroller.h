/* ============================================================
* AutoScroll - Autoscroll for QupZilla
* Copyright (C) 2014  David Rosca <nowrep@gmail.com>
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
#ifndef FRAMESCROLLER_H
#define FRAMESCROLLER_H

#include <QObject>

class QTimer;

class WebPage;

class FrameScroller : public QObject
{
    Q_OBJECT

public:
    explicit FrameScroller(QObject* parent = 0);

    void setPage(WebPage *page);

    double scrollDivider() const;
    void setScrollDivider(double divider);

    void startScrolling(int lengthX, int lengthY);
    void stopScrolling();

private slots:
    void scrollStep();

private:
    WebPage *m_page;
    QTimer* m_timer;

    int m_lengthX;
    int m_lengthY;
    double m_divider;
};

#endif // FRAMESCROLLER_H
