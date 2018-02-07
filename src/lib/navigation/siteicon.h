/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018 David Rosca <nowrep@gmail.com>
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
#ifndef SITEICON_H
#define SITEICON_H

#include "qzcommon.h"
#include "toolbutton.h"

class QTimer;

class LocationBar;
class WebView;
class BrowserWindow;

class QUPZILLA_EXPORT SiteIcon : public ToolButton
{
    Q_OBJECT

public:
    explicit SiteIcon(LocationBar *parent);

    void setBrowserWindow(BrowserWindow *window);
    void setWebView(WebView* view);
    void setIcon(const QIcon &icon);

private slots:
    void updateIcon();
    void popupClosed();

private:
    void contextMenuEvent(QContextMenuEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);

    bool showPopup();

    BrowserWindow* m_window;
    LocationBar* m_locationBar;
    WebView* m_view;
    QTimer* m_updateTimer;

    QPoint m_dragStartPosition;
    QIcon m_icon;
};

#endif // SITEICON_H
