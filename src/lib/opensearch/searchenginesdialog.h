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
#ifndef SEARCHENGINESDIALOG_H
#define SEARCHENGINESDIALOG_H

#include "qzcommon.h"
#include "searchenginesmanager.h"

#include <QDialog>

class QTreeWidgetItem;

namespace Ui
{
class SearchEnginesDialog;
}

class QUPZILLA_EXPORT SearchEnginesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SearchEnginesDialog(QWidget* parent = 0);
    ~SearchEnginesDialog();

public slots:
    void accept();

private slots:
    void addEngine();
    void removeEngine();
    void editEngine();
    void setDefaultEngine();

    void moveUp();
    void moveDown();

    void defaults();

private:
    enum TreeRole { EngineRole = Qt::UserRole, DefaultRole = Qt::UserRole + 1 };

    bool isDefaultEngine(QTreeWidgetItem* item);
    SearchEngine getEngine(QTreeWidgetItem* item);

    void setEngine(QTreeWidgetItem* item, SearchEngine engine);
    void changeItemToDefault(QTreeWidgetItem* item, bool isDefault);

    void reloadEngines();

    void showEvent(QShowEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void resizeViewHeader();

    Ui::SearchEnginesDialog* ui;
    SearchEnginesManager* m_manager;
};

#endif // SEARCHENGINESDIALOG_H
