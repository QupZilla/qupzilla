/* ============================================================
* StatusBarIcons - Extra icons in statusbar for QupZilla
* Copyright (C) 2013  David Rosca <nowrep@gmail.com>
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
#ifndef SBI_NETWORKICONDIALOG_H
#define SBI_NETWORKICONDIALOG_H

#include <QDialog>

namespace Ui
{
class SBI_NetworkIconDialog;
}

class SBI_NetworkIcon;

class SBI_NetworkIconDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SBI_NetworkIconDialog(QWidget* parent = 0);
    ~SBI_NetworkIconDialog();

private slots:
    void addProxy();
    void removeProxy();
    void saveProxy();

    void showProxy(const QString &name);

private:
    void updateWidgets();

    Ui::SBI_NetworkIconDialog* ui;
};

#endif // SBI_NETWORKICONDIALOG_H
