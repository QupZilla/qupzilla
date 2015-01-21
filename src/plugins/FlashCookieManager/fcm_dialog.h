/* ============================================================
* FlashCookieManager plugin for QupZilla
* Copyright (C) 2014  S. Razi Alavizadeh <s.r.alavizadeh@gmail.com>
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
*
* some codes and ideas are taken from cookiemanager.cpp and cookiemanager.ui
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
#ifndef FCM_DIALOG_H
#define FCM_DIALOG_H

#include <QDialog>


namespace Ui
{
class FCM_Dialog;
}

class QTreeWidgetItem;

class BrowserWindow;
class FCM_Plugin;

class FCM_Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit FCM_Dialog(FCM_Plugin* manager, QWidget* parent = 0);
    ~FCM_Dialog();

    void refreshView(bool forceReload = false);
    void showPage(int index);

private slots:
    void currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* parent);
    void removeCookie();
    void removeAll();

    void refreshFlashCookiesTree();
    void refreshFilters();

    void addWhitelist();
    void addWhitelist(const QString &origin);
    void removeWhitelist();
    void addBlacklist();
    void addBlacklist(const QString &origin);
    void removeBlacklist();

    void deletePressed();
    void autoModeChanged(bool state);

    void filterString(const QString &string);

    void reloadFromDisk();
    void selectFlashDataPath();
    void cookieTreeContextMenuRequested(const QPoint &pos);

private:
    void closeEvent(QCloseEvent* e);
    void keyPressEvent(QKeyEvent* e);

    Ui::FCM_Dialog* ui;

    FCM_Plugin* m_manager;
};

#endif // FCM_DIALOG_H
