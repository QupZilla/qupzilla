/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  nowrep
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
#include <QVBoxLayout>

class DockTitleBarWidget;
class SideBar : public QWidget
{
    Q_OBJECT
public:
    enum SideWidget { None = 0, Bookmarks, History, RSS };

    explicit SideBar(QWidget* parent = 0);
    ~SideBar();

    void showBookmarks();
    void showHistory();
    void showRSS();
    SideWidget activeWidget() { return m_activeWidget; }

signals:

public slots:
    void close();

private:
    void setWidget(QWidget* widget);

    QVBoxLayout* m_layout;
    DockTitleBarWidget* m_titleBar;
    SideWidget m_activeWidget;
};

#endif // SIDEBAR_H
