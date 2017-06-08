/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2017 David Rosca <nowrep@gmail.com>
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
#ifndef SESSIONMANAGERDIALOG_H
#define SESSIONMANAGERDIALOG_H

#include <QDialog>

namespace Ui {
class SessionManagerDialog;
}

class QTreeWidgetItem;

class SessionManagerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SessionManagerDialog(QWidget *parent = 0);
    ~SessionManagerDialog();

private:
    enum Roles {
        SessionFileRole = Qt::UserRole + 10,
        IsBackupSessionRole,
        IsActiveSessionRole,
        IsDefaultSessionRole
    };

    void newSession();
    void renameSession();
    void cloneSession();
    void deleteSession();
    void switchToSession();

    void refresh();
    void updateButtons();
    void updateItem(QTreeWidgetItem *item);

    void showEvent(QShowEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void resizeViewHeader();

    Ui::SessionManagerDialog *ui;
};

#endif // SESSIONMANAGERDIALOG_H
