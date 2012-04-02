/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
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
#ifndef PLUGINSLIST_H
#define PLUGINSLIST_H

#include <QWidget>

#include "qz_namespace.h"

namespace Ui
{
class PluginsList;
}

class QListWidgetItem;

class QT_QUPZILLA_EXPORT PluginsList : public QWidget
{
    Q_OBJECT

public:
    explicit PluginsList(QWidget* parent = 0);
    ~PluginsList();
    void save();

private slots:
    //App extension
    void settingsClicked();
    void currentChanged(QListWidgetItem* item);
    void itemChanged(QListWidgetItem* item);
    void allowAppPluginsChanged(bool state);

    void refresh();

    //WebKit plugins
    void addWhitelist();
    void removeWhitelist();
    void allowC2FChanged(bool state);

private:;

    Ui::PluginsList* ui;
};

#endif // PLUGINSLIST_H
