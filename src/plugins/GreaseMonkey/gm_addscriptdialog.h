/* ============================================================
* GreaseMonkey plugin for QupZilla
* Copyright (C) 2012-2014  David Rosca <nowrep@gmail.com>
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
#ifndef GM_ADDSCRIPTDIALOG_H
#define GM_ADDSCRIPTDIALOG_H

#include <QDialog>

namespace Ui
{
class GM_AddScriptDialog;
}

class GM_Script;
class GM_Manager;

class GM_AddScriptDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GM_AddScriptDialog(GM_Manager *manager, GM_Script *script, QWidget *parent = Q_NULLPTR);
    ~GM_AddScriptDialog();

private slots:
    void showSource();

    void accepted();

private:
    Ui::GM_AddScriptDialog* ui;

    GM_Manager* m_manager;
    GM_Script* m_script;
};

#endif // GM_ADDSCRIPTDIALOG_H
