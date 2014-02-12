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
#ifndef POPUPWEBVIEW_H
#define POPUPWEBVIEW_H

#include "qz_namespace.h"
#include "webview.h"

class PopupWebPage;
class Menu;

class QT_QUPZILLA_EXPORT PopupWebView : public WebView
{
    Q_OBJECT
public:
    explicit PopupWebView(QWidget* parent = 0);

    void setWebPage(PopupWebPage* page);
    PopupWebPage* webPage();

    QWidget* overlayForJsAlert();
    void loadInNewTab(const QNetworkRequest &req, QNetworkAccessManager::Operation op,
                      const QByteArray &data, Qz::NewTabPositionFlags position);

signals:

public slots:
    void closeView();
    void inspectElement();

private:
    void contextMenuEvent(QContextMenuEvent* event);

    PopupWebPage* m_page;
    Menu* m_menu;
};

#endif // POPUPWEBVIEW_H
