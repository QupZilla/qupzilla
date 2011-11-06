/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca
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
#include <QHBoxLayout>
#include <QMenu>
#include <QSplitter>

class ToolButton;
class WebSearchBar;
class QupZilla;
class ReloadStopButton;
class NavigationBar : public QWidget
{
    Q_OBJECT
public:
    explicit NavigationBar(QupZilla* mainClass, QWidget* parent = 0);
    ~NavigationBar();

    void setSplitterSizes(int locationBar, int websearchBar);

    void showReloadButton();
    void showStopButton();

    inline ToolButton* buttonBack() { return m_buttonBack; }
    inline ToolButton* buttonNext() { return m_buttonNext; }
    inline ToolButton* buttonHome() { return m_buttonHome; }
    inline ToolButton* buttonAddTab() { return m_buttonAddTab; }
    inline ToolButton* buttonSuperMenu() { return m_supMenu; }
    inline ToolButton* buttonExitFullscreen() { return m_exitFullscreen; }
    inline WebSearchBar* searchLine() { return m_searchLine; }
    inline QSplitter* splitter() { return m_navigationSplitter; }

signals:

public slots:
    void refreshHistory();

    void goBack();
    void goForward();

private slots:
    void aboutToShowHistoryNextMenu();
    void aboutToShowHistoryBackMenu();

    void goAtHistoryIndex();
    void clearHistory();

private:
    QupZilla* p_QupZilla;

    QHBoxLayout* m_layout;
    QSplitter* m_navigationSplitter;
    ToolButton* m_buttonBack;
    ToolButton* m_buttonNext;
    ToolButton* m_buttonHome;
    ToolButton* m_buttonAddTab;
    ToolButton* m_supMenu;
    ToolButton* m_exitFullscreen;
    ReloadStopButton* m_reloadStop;

    QMenu* m_menuBack;
    QMenu* m_menuForward;

    WebSearchBar* m_searchLine;
};

#endif // NAVIGATIONBAR_H
