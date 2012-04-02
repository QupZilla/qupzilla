/* ============================================================
* Access Keys Navigation plugin for QupZilla
* Copyright (C) 2012  David Rosca <nowrep@gmail.com>
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
#ifndef AKN_SETTINGS_H
#define AKN_SETTINGS_H

#include <QDialog>

namespace Ui
{
class AKN_Settings;
}

class AKN_Handler;

class AKN_Settings : public QDialog
{
    Q_OBJECT

public:
    explicit AKN_Settings(AKN_Handler* handler, QWidget* parent = 0);
    ~AKN_Settings();

private slots:
    void dialogAccepted();
    void showLicence();

private:
    Ui::AKN_Settings* ui;

    AKN_Handler* m_handler;
    QString m_settingsFile;
};

#endif // AKN_SETTINGS_H
