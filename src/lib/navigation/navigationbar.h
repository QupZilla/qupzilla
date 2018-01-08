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
#ifndef NAVIGATIONBAR_H
#define NAVIGATIONBAR_H

#include <QWidget>

#include "qzcommon.h"

class QUrl;
class QHBoxLayout;
class QSplitter;
class QWebEngineHistoryItem;

class ToolButton;
class WebSearchBar;
class BrowserWindow;
class ReloadStopButton;
class Menu;
class AbstractButtonInterface;
class TabbedWebView;

class QUPZILLA_EXPORT NavigationBar : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int layoutMargin READ layoutMargin WRITE setLayoutMargin)
    Q_PROPERTY(int layoutSpacing READ layoutSpacing WRITE setLayoutSpacing)

public:
    explicit NavigationBar(BrowserWindow* window);
    ~NavigationBar();

    void setSplitterSizes(int locationBar, int websearchBar);

    void setCurrentView(TabbedWebView *view);

    void showReloadButton();
    void showStopButton();

    void enterFullScreen();
    void leaveFullScreen();

    WebSearchBar* webSearchBar() { return m_searchLine; }
    QSplitter* splitter() { return m_navigationSplitter; }

    void setSuperMenuVisible(bool visible);

    int layoutMargin() const;
    void setLayoutMargin(int margin);

    int layoutSpacing() const;
    void setLayoutSpacing(int spacing);

    void addWidget(QWidget *widget, const QString &id, const QString &name);
    void removeWidget(const QString &id);

    void addToolButton(AbstractButtonInterface *button);
    void removeToolButton(AbstractButtonInterface *button);

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
    void aboutToShowToolsMenu();

    void loadHistoryIndex();
    void loadHistoryIndexInNewTab(int index = -1);

    void clearHistory();
    void contextMenuRequested(const QPoint &pos);
    void openConfigurationDialog();
    void toolActionActivated();

private:
    void loadSettings();
    void reloadLayout();
    void loadHistoryItem(const QWebEngineHistoryItem &item);
    void loadHistoryItemInNewTab(const QWebEngineHistoryItem &item);

    BrowserWindow* m_window;
    QHBoxLayout* m_layout;
    QSplitter* m_navigationSplitter;
    WebSearchBar* m_searchLine;

    Menu* m_menuBack;
    Menu* m_menuForward;
    ToolButton* m_buttonBack;
    ToolButton* m_buttonForward;
    ReloadStopButton* m_reloadStop;
    Menu *m_menuTools;
    ToolButton* m_supMenu;
    ToolButton *m_exitFullscreen;

    struct WidgetData {
        QString id;
        QString name;
        QWidget *widget = nullptr;
        AbstractButtonInterface *button = nullptr;
    };

    QStringList m_layoutIds;
    QHash<QString, WidgetData> m_widgets;

    friend class NavigationBarConfigDialog;
};

#endif // NAVIGATIONBAR_H
