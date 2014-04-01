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
#ifndef TABBEDWEBVIEW_H
#define TABBEDWEBVIEW_H

#include "qzcommon.h"
#include "webview.h"

class QLabel;
class QHostInfo;

class BrowserWindow;
class TabWidget;
class WebPage;
class WebTab;
class Menu;

class QUPZILLA_EXPORT TabbedWebView : public WebView
{
    Q_OBJECT
public:
    explicit TabbedWebView(BrowserWindow* window, WebTab* webTab);
    ~TabbedWebView();

    void setWebPage(WebPage* pag);

    WebTab* webTab() const;
    TabWidget* tabWidget() const;

    QString getIp() const;
    int tabIndex() const;

    BrowserWindow* mainWindow() const;
    void moveToWindow(BrowserWindow* window);

    QWidget* overlayWidget();

signals:
    void wantsCloseTab(int);
    void ipChanged(QString);
    void changed();

public slots:
    void titleChanged();
    void setAsCurrentTab();

    void slotLoadStarted();
    void loadProgress(int prog);

    void userLoadAction(const LoadRequest &req);

    void closeView();
    void openNewTab();
    void loadInNewTab(const LoadRequest &req, Qz::NewTabPositionFlags position);


private slots:
    void slotLoadFinished();
    void urlChanged(const QUrl &url);
    void linkHovered(const QString &link, const QString &title, const QString &content);
    void setIp(const QHostInfo &info);

    void inspectElement();

private:
    void contextMenuEvent(QContextMenuEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

    BrowserWindow* m_window;
    WebTab* m_webTab;
    Menu* m_menu;

    QString m_currentIp;

};

#endif // TABBEDWEBVIEW_H
