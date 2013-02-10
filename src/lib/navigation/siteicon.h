/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2013  David Rosca <nowrep@gmail.com>
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

#include "qz_namespace.h"
#include "toolbutton.h"

class LocationBar;
class WebView;
class QupZilla;

class QT_QUPZILLA_EXPORT SiteIcon : public ToolButton
{
    Q_OBJECT

public:
    explicit SiteIcon(QupZilla* window, LocationBar* parent);

    void setWebView(WebView* view);

private slots:
    void iconClicked();

private:
    void contextMenuEvent(QContextMenuEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);

    QupZilla* p_QupZilla;
    LocationBar* m_locationBar;
    WebView* m_view;

    QPoint m_dragStartPosition;
};

#endif // SITEICON_H
