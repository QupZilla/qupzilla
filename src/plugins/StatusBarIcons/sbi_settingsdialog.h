/* ============================================================
* StatusBarIcons - Extra icons in statusbar for QupZilla
* Copyright (C) 2013-2014  David Rosca <nowrep@gmail.com>
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
#ifndef SBI_SETTINGSDIALOG_H
#define SBI_SETTINGSDIALOG_H

#include <QDialog>

namespace Ui
{
class SBI_SettingsDialog;
}

class SBI_IconsManager;

class SBI_SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SBI_SettingsDialog(SBI_IconsManager* manager, QWidget* parent = 0);
    ~SBI_SettingsDialog();

private slots:
    void saveSettings();

private:
    Ui::SBI_SettingsDialog* ui;

    SBI_IconsManager* m_manager;
};

#endif // SBI_SETTINGSDIALOG_H
