/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2015  David Rosca <nowrep@gmail.com>
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
#ifndef POPUPWEBVIEW_H
#define POPUPWEBVIEW_H

#include <QPointer>

#include "qzcommon.h"
#include "webview.h"

class Menu;
class LoadRequest;
class WebInspector;

class QUPZILLA_EXPORT PopupWebView : public WebView
{
    Q_OBJECT
public:
    explicit PopupWebView(QWidget* parent = 0);

    QWidget* overlayWidget() Q_DECL_OVERRIDE;
    void loadInNewTab(const LoadRequest &req, Qz::NewTabPositionFlags position) Q_DECL_OVERRIDE;

    void closeView() Q_DECL_OVERRIDE;
    bool isFullScreen() Q_DECL_OVERRIDE;
    void requestFullScreen(bool enable) Q_DECL_OVERRIDE;

public slots:
    void inspectElement();

private:
    void _contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;

    Menu* m_menu;
    QPointer<WebInspector> m_inspector;
};

#endif // POPUPWEBVIEW_H
