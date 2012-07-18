/* ============================================================
* Personal Information Manager plugin for QupZilla
* Copyright (C) 2012  David Rosca <nowrep@gmail.com>
* Copyright (C) 2012  Mladen Pejaković <pejakm@gmail.com>
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
#ifndef PIM_SETTINGS_H
#define PIM_SETTINGS_H

#include <QDialog>

namespace Ui
{
class PIM_Settings;
}

class PIM_Handler;

class PIM_Settings : public QDialog
{
    Q_OBJECT

public:
    explicit PIM_Settings(const QString &settingsFile, QWidget* parent = 0);
    ~PIM_Settings();

private slots:
    void dialogAccepted();

private:
    Ui::PIM_Settings* ui;

    QString m_settingsFile;
};

#endif // PIM_SETTINGS_H
