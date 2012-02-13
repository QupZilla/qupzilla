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
#ifndef COOKIEMANAGER_H
#define COOKIEMANAGER_H

#include <QWidget>
#include <QTimer>
#include <QNetworkCookie>
#include <QTreeWidgetItem>

namespace Ui
{
class CookieManager;
}

class QupZilla;
class CookieManager : public QWidget
{
    Q_OBJECT

public:
    explicit CookieManager(QWidget* parent = 0);
    ~CookieManager();

    void refreshTable(bool refreshCookieJar = true);

private slots:
    void currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* parent);
    void removeCookie();
    void removeAll();
    void search();

    void slotRefreshTable();

private:
    Ui::CookieManager* ui;

    QList<QNetworkCookie> m_cookies;
    bool m_refreshCookieJar;
};

#endif // COOKIEMANAGER_H
