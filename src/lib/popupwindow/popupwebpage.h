/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
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
#ifndef POPUPWEBPAGE_H
#define POPUPWEBPAGE_H

#include "qzcommon.h"
#include "webpage.h"

class BrowserWindow;

class QUPZILLA_EXPORT PopupWebPage : public WebPage
{
    Q_OBJECT
public:
    explicit PopupWebPage(WebWindowType type, BrowserWindow* window);

    BrowserWindow* mainWindow() const;

private slots:
    void slotGeometryChangeRequested(const QRect &rect);
    void slotMenuBarVisibilityChangeRequested(bool visible);
    void slotStatusBarVisibilityChangeRequested(bool visible);
    void slotToolBarVisibilityChangeRequested(bool visible);

    void slotLoadStarted();
    void slotLoadProgress(int prog);
    void slotLoadFinished(bool state);

    void checkBehaviour();

private:
    BrowserWindow* m_window;
    QWebEnginePage::WebWindowType m_type;
    bool m_createNewWindow;

    bool m_menuBarVisible;
    bool m_statusBarVisible;
    bool m_toolBarVisible;
    QRect m_geometry;

    bool m_isLoading;
    bool m_progress;
};

#endif // POPUPWEBPAGE_H
