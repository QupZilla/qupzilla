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
#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <QWidget>
#include <QHash>
#include <QPointer>

#include "qzcommon.h"

class QVBoxLayout;
class QMenu;

class DockTitleBarWidget;
class SideBarInterface;
class SideBarManager;
class BrowserWindow;

class QUPZILLA_EXPORT SideBar : public QWidget
{
    Q_OBJECT
public:
    explicit SideBar(SideBarManager* manager, BrowserWindow* window);

    void showBookmarks();
    void showHistory();

    void setTitle(const QString &title);
    void setWidget(QWidget* widget);

public slots:
    void close();

private:
    BrowserWindow* m_window;
    QVBoxLayout* m_layout;
    DockTitleBarWidget* m_titleBar;
    SideBarManager* m_manager;
};

class QUPZILLA_EXPORT SideBarManager : public QObject
{
    Q_OBJECT
public:
    explicit SideBarManager(BrowserWindow* parent);

    QString activeSideBar() const;

    void createMenu(QMenu* menu);

    void showSideBar(const QString &id, bool toggle = true);
    void sideBarRemoved(const QString &id);
    void closeSideBar();

    static QHash<QString, QPointer<SideBarInterface> > s_sidebars;
    static void addSidebar(const QString &id, SideBarInterface* interface);
    static void removeSidebar(SideBarInterface *interface);

private slots:
    void slotShowSideBar();

private:
    BrowserWindow* m_window;
    QPointer<SideBar> m_sideBar;
    QMenu* m_menu;

    QString m_activeBar;
};

#endif // SIDEBAR_H
