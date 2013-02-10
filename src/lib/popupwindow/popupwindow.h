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
#ifndef POPUPWINDOW_H
#define POPUPWINDOW_H

#include <QWidget>
#include <QPointer>

#include "qz_namespace.h"

class QVBoxLayout;
class QStatusBar;
class QMenuBar;
class QMenu;

class PopupWebView;
class PopupWebPage;
class PopupStatusBarMessage;
class PopupLocationBar;
class ProgressBar;
class SearchToolBar;

class QT_QUPZILLA_EXPORT PopupWindow : public QWidget
{
    Q_OBJECT
public:
    explicit PopupWindow(PopupWebView* view);

    QStatusBar* statusBar();
    PopupWebView* webView();

public slots:
    void setWindowGeometry(QRect newRect);
    void setStatusBarVisibility(bool visible);
    void setMenuBarVisibility(bool visible);
    void setToolBarVisibility(bool visible);

private slots:
    void titleChanged();
    void showNotification(QWidget* notif);
    void showStatusBarMessage(const QString &message);

    void loadStarted();
    void loadProgress(int value);
    void loadFinished();

    void editUndo();
    void editRedo();
    void editCut();
    void editCopy();
    void editPaste();
    void editSelectAll();

    void aboutToShowEditMenu();
    void aboutToHideEditMenu();

    void savePageScreen();
    void searchOnPage();

private:
    void closeEvent(QCloseEvent* event);

    PopupWebView* m_view;
    PopupWebPage* m_page;
    PopupLocationBar* m_locationBar;
    PopupStatusBarMessage* m_statusBarMessage;
    ProgressBar* m_progressBar;

    QVBoxLayout* m_layout;
    QStatusBar* m_statusBar;
    QMenuBar* m_menuBar;
    QMenu* m_menuEdit;
    QMenu* m_menuView;
    QAction* m_actionReload;
    QAction* m_actionStop;
    QPointer<SearchToolBar> m_search;
};

#endif // POPUPWINDOW_H
