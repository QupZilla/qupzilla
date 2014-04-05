/* ============================================================
* Mouse Gestures plugin for QupZilla
* Copyright (C) 2012-2014  David Rosca <nowrep@gmail.com>
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
#ifndef MOUSEGESTURES_H
#define MOUSEGESTURES_H

#include <QObject>
#include <QPointer>

class QMouseEvent;

class WebView;
class QjtMouseGestureFilter;
class MouseGesturesSettingsDialog;

class MouseGestures : public QObject
{
    Q_OBJECT
public:
    explicit MouseGestures(QObject* parent = 0);
    ~MouseGestures();

    bool mousePress(QObject* obj, QMouseEvent* event);
    bool mouseRelease(QObject* obj, QMouseEvent* event);
    bool mouseMove(QObject* obj, QMouseEvent* event);

    void showSettings(QWidget* parent);
    void unloadPlugin();

private slots:
    void upGestured();
    void downGestured();
    void leftGestured();
    void rightGestured();

    void downRightGestured();
    void downLeftGestured();

    void upDownGestured();
    void upLeftGestured();
    void upRightGestured();

private:
    QjtMouseGestureFilter* m_filter;
    QPointer<MouseGesturesSettingsDialog> m_settings;
    QPointer<WebView> m_view;
    bool m_blockNextRightMouseRelease;
    bool m_blockNextLeftMouseRelease;

    bool m_enableRockerNavigation;
};

#endif // MOUSEGESTURES_H
