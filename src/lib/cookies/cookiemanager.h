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
#ifndef COOKIEMANAGER_H
#define COOKIEMANAGER_H

#include <QDialog>

#include "qzcommon.h"

namespace Ui
{
class CookieManager;
}

class QTreeWidgetItem;

class BrowserWindow;

class QUPZILLA_EXPORT CookieManager : public QDialog
{
    Q_OBJECT

public:
    explicit CookieManager(QWidget* parent = 0);
    ~CookieManager();

    void refreshTable();

private slots:
    void currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* parent);
    void removeCookie();
    void removeAll();
    void blockCurrentHostAndRemoveCookie();

    void slotRefreshTable();
    void slotRefreshFilters();

    void addWhitelist();
    void removeWhitelist();
    void addBlacklist();
    void removeBlacklist();

    void deletePressed();
    void saveCookiesChanged(bool state);

    void filterString(const QString &string);

private:
    void closeEvent(QCloseEvent* e);
    void keyPressEvent(QKeyEvent* e);
    void addBlacklist(const QString &server);

    Ui::CookieManager* ui;

    bool m_refreshCookieJar;
};

#endif // COOKIEMANAGER_H
