/* ============================================================
* GreaseMonkey plugin for QupZilla
* Copyright (C) 2012-2013  David Rosca <nowrep@gmail.com>
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
#ifndef GM_SETTINGS_H
#define GM_SETTINGS_H

#include <QDialog>

namespace Ui
{
class GM_Settings;
}

class QListWidgetItem;

class GM_Manager;
class GM_Script;

class GM_Settings : public QDialog
{
    Q_OBJECT

public:
    explicit GM_Settings(GM_Manager* manager, QWidget* parent = 0);
    ~GM_Settings();

private slots:
    void showItemInfo(QListWidgetItem* item);
    void removeItem(QListWidgetItem* item);

    void itemChanged(QListWidgetItem* item);

    void openScriptsDirectory();
    void newScript();
    void openUserJs();

    void loadScripts();

private:
    inline GM_Script* getScript(QListWidgetItem* item);

    Ui::GM_Settings* ui;
    GM_Manager* m_manager;
};

#endif // GM_SETTINGS_H
