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
#ifndef NAVIGATIONBAR_H
#define NAVIGATIONBAR_H

#include <QWidget>

#include "qzcommon.h"

class QHBoxLayout;
class QSplitter;
class QWebHistoryItem;

class ToolButton;
class WebSearchBar;
class BrowserWindow;
class ReloadStopButton;
class Menu;
class QUrl;

class QUPZILLA_EXPORT NavigationBar : public QWidget
{
    Q_OBJECT
public:
    explicit NavigationBar(BrowserWindow* window);

    Q_PROPERTY(int layoutMargin READ layoutMargin WRITE setLayoutMargin)
    Q_PROPERTY(int layoutSpacing READ layoutSpacing WRITE setLayoutSpacing)

    void setSplitterSizes(int locationBar, int websearchBar);

    void showReloadButton();
    void showStopButton();

    ToolButton* buttonBack() { return m_buttonBack; }
    ToolButton* buttonForward() { return m_buttonForward; }
    ToolButton* buttonHome() { return m_buttonHome; }
    ToolButton* buttonAddTab() { return m_buttonAddTab; }
    ToolButton* buttonExitFullscreen() { return m_exitFullscreen; }
    ReloadStopButton* buttonReloadStop() { return m_reloadStop; }
    WebSearchBar* webSearchBar() { return m_searchLine; }
    QSplitter* splitter() { return m_navigationSplitter; }

    void setSuperMenuVisible(bool visible);

    int layoutMargin() const;
    void setLayoutMargin(int margin);

    int layoutSpacing() const;
    void setLayoutSpacing(int spacing);

signals:

public slots:
    void refreshHistory();

    void stop();
    void reload();
    void goBack();
    void goBackInNewTab();
    void goForward();
    void goForwardInNewTab();

private slots:
    void aboutToShowHistoryNextMenu();
    void aboutToShowHistoryBackMenu();

    void loadHistoryIndex();
    void loadHistoryIndexInNewTab(int index = -1);

    void clearHistory();
    void contextMenuRequested(const QPoint &pos);

private:
    QString titleForUrl(QString title, const QUrl &url);
    QIcon iconForPage(const QUrl &url, const QIcon &sIcon);

    void loadHistoryItem(const QWebHistoryItem &item);
    void loadHistoryItemInNewTab(const QWebHistoryItem &item);

    BrowserWindow* m_window;

    QHBoxLayout* m_layout;
    QSplitter* m_navigationSplitter;
    ToolButton* m_buttonBack;
    ToolButton* m_buttonForward;
    ToolButton* m_buttonHome;
    ToolButton* m_buttonAddTab;
    ToolButton* m_supMenu;
    ToolButton* m_exitFullscreen;
    ReloadStopButton* m_reloadStop;

    Menu* m_menuBack;
    Menu* m_menuForward;

    WebSearchBar* m_searchLine;
};

#endif // NAVIGATIONBAR_H
