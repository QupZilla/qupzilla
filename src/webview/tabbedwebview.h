/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
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

#include <QDebug>
#include <QTabWidget>
#include <QContextMenuEvent>
#include <QLabel>
#include <QHostInfo>

#include "webview.h"

class QupZilla;
class TabWidget;
class WebPage;
class WebTab;
class TabbedWebView : public WebView
{
    Q_OBJECT
public:
    explicit TabbedWebView(QupZilla* mainClass, WebTab* webTab);
    ~TabbedWebView();

    void setWebPage(WebPage* page);

    WebPage* webPage() const;
    WebTab* webTab() const;

    QString getIp() const;
    QLabel* animationLoading(int index, bool addMovie);

    int tabIndex() const;
    bool hasRss() { return m_hasRss; }

    QWidget* overlayForJsAlert();

    void disconnectObjects();

signals:
    void wantsCloseTab(int);
    void ipChanged(QString);
    void rssChanged(bool);
    void changed();

public slots:
    void titleChanged();

    void stop();
    void showIcon();

    void slotLoadStarted();
    void loadingProgress(int prog);

    void closeView();

private slots:
    void trackMouse(bool state) { m_mouseTrack = state; }
    void slotLoadFinished();
    void urlChanged(const QUrl &url);
    void linkHovered(const QString &link, const QString &title, const QString &content);
    void getFocus(const QUrl &urla);
    void stopAnimation();
    void setIp(const QHostInfo &info);
    void checkRss();
    void slotIconChanged();

    void inspectElement();
    void openUrlInNewTab(const QUrl &url, Qz::NewTabPositionFlag position);

private:
    void contextMenuEvent(QContextMenuEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

    bool isCurrent();

    QupZilla* p_QupZilla;
    TabWidget* m_tabWidget;

    QString m_hoveredLink;
    QString m_currentIp;

    WebPage* m_page;
    WebTab* m_webTab;
    QMenu* m_menu;

    bool m_mouseTrack;
    bool m_navigationVisible;

    bool m_hasRss;
    bool m_rssChecked;

};

#endif // TABBEDWEBVIEW_H
