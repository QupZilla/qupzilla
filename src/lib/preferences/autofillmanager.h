/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2018  David Rosca <nowrep@gmail.com>
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
#ifndef AUTOFILLMANAGER_H
#define AUTOFILLMANAGER_H

#include <QWidget>

#include "qzcommon.h"

class PasswordManager;

namespace Ui
{
class AutoFillManager;
}

class QUPZILLA_EXPORT AutoFillManager : public QWidget
{
    Q_OBJECT

public:
    explicit AutoFillManager(QWidget* parent = 0);
    ~AutoFillManager();

    void showExceptions();

private slots:
    void loadPasswords();
    void changePasswordBackend();
    void showBackendOptions();

    void removePass();
    void removeAllPass();
    void editPass();
    void showPasswords();

    void copyPassword();
    void copyUsername();

    void removeExcept();
    void removeAllExcept();

    void importPasswords();
    void exportPasswords();

    void slotImportPasswords();
    void slotExportPasswords();

    void currentPasswordBackendChanged();
    void passwordContextMenu(const QPoint &pos);

private:
    Ui::AutoFillManager* ui;

    PasswordManager* m_passwordManager;
    QString m_fileName;
    bool m_passwordsShown;
};

#endif // AUTOFILLMANAGER_H
