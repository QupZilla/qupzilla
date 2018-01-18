/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2014  David Rosca <nowrep@gmail.com>
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
#ifndef BOOKMARKSEXPORTDIALOG_H
#define BOOKMARKSEXPORTDIALOG_H

#include <QDialog>

#include "qzcommon.h"

namespace Ui
{
class BookmarksExportDialog;
}

class BookmarksExporter;

class QUPZILLA_EXPORT BookmarksExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BookmarksExportDialog(QWidget* parent = 0);
    ~BookmarksExportDialog();

private slots:
    void setPath();
    void exportBookmarks();

private:
    void init();

    Ui::BookmarksExportDialog* ui;
    QList<BookmarksExporter*> m_exporters;
    BookmarksExporter* m_currentExporter;
};

#endif // BOOKMARKSEXPORTDIALOG_H
