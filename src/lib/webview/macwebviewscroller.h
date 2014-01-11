/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2013-2014  David Rosca <nowrep@gmail.com>
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
#ifndef MACWEBVIEWSCROLLER_H
#define MACWEBVIEWSCROLLER_H

#include <QObject>
#include <QPoint>

class QWheelEvent;
class QWebView;

class MacWebViewScroller : public QObject
{
    Q_OBJECT
public:
    explicit MacWebViewScroller(QWebView* view);

    bool eventFilter(QObject* obj, QEvent* event);

private slots:
    void sendWheelEvent();

private:
    QWebView* m_view;

    bool m_timerRunning;
    int m_delta;
    QPoint m_pos;
    QPoint m_globalPos;

};

#endif // MACWEBVIEWSCROLLER_H
