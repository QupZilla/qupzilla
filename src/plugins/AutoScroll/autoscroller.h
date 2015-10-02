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
#ifndef AUTOSCROLLER_H
#define AUTOSCROLLER_H

#include <QObject>
#include <QPoint>

class QMouseEvent;
class QWheelEvent;
class QLabel;
class QRect;

class WebView;
class FrameScroller;

class AutoScroller : public QObject
{
    Q_OBJECT
public:
    explicit AutoScroller(const QString &settingsFile, QObject* parent = 0);
    ~AutoScroller();

    bool mouseMove(QObject* obj, QMouseEvent* event);
    bool mousePress(QObject* obj, QMouseEvent* event);
    bool mouseRelease(QObject* obj, QMouseEvent* event);
    bool wheel(QObject* obj, QWheelEvent *event);

    double scrollDivider() const;
    void setScrollDivider(double divider);

private:
    bool eventFilter(QObject* obj, QEvent* event);

    bool showIndicator(WebView* view, const QPoint &pos);
    void stopScrolling();

    QRect indicatorGlobalRect() const;

    WebView* m_view;
    QLabel* m_indicator;
    FrameScroller* m_frameScroller;
    QString m_settingsFile;
};

#endif // AUTOSCROLLER_H
