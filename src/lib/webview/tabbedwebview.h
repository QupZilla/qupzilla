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
#ifndef TABBEDWEBVIEW_H
#define TABBEDWEBVIEW_H

#include "qz_namespace.h"
#include "webview.h"

class QLabel;
class QHostInfo;

class QupZilla;
class TabWidget;
class WebPage;
class WebTab;
class Menu;

class QT_QUPZILLA_EXPORT TabbedWebView : public WebView
{
    Q_OBJECT
public:
    explicit TabbedWebView(QupZilla* mainClass, WebTab* webTab);
    ~TabbedWebView();

    void setWebPage(WebPage* pag);

    WebTab* webTab() const;
    TabWidget* tabWidget() const;

    QString getIp() const;
    int tabIndex() const;

    QupZilla* mainWindow() const;
    void moveToWindow(QupZilla* window);

    QWidget* overlayForJsAlert();
    void disconnectObjects();

signals:
    void wantsCloseTab(int);
    void ipChanged(QString);
    void changed();

public slots:
    void titleChanged();
    void setAsCurrentTab();

    void stop();
    void showIcon();

    void slotLoadStarted();
    void loadProgress(int prog);

    void userLoadAction(const QUrl &url);

    void closeView();
    void loadInNewTab(const QNetworkRequest &req, QNetworkAccessManager::Operation op,
                      const QByteArray &data, Qz::NewTabPositionFlag position);
    void openNewTab();

private slots:
    void trackMouse(bool state) { m_mouseTrack = state; }
    void slotLoadFinished();
    void urlChanged(const QUrl &url);
    void linkHovered(const QString &link, const QString &title, const QString &content);
    void setIp(const QHostInfo &info);

    void inspectElement();

private:
    void contextMenuEvent(QContextMenuEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

    bool isCurrent();

    QupZilla* p_QupZilla;
    WebTab* m_webTab;
    Menu* m_menu;

    QString m_currentIp;
    bool m_mouseTrack;

};

#endif // TABBEDWEBVIEW_H
