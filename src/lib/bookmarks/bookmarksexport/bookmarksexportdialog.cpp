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
#include "bookmarksexportdialog.h"
#include "ui_bookmarksexportdialog.h"
#include "htmlexporter.h"
#include "mainapplication.h"
#include "bookmarks.h"

#include <QMessageBox>

BookmarksExportDialog::BookmarksExportDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::BookmarksExportDialog)
    , m_currentExporter(0)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    init();

    connect(ui->chooseOutput, SIGNAL(clicked()), this, SLOT(setPath()));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(exportBookmarks()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(close()));
}

BookmarksExportDialog::~BookmarksExportDialog()
{
    delete ui;
}

void BookmarksExportDialog::setPath()
{
    Q_ASSERT(m_currentExporter);

    ui->output->setText(m_currentExporter->getPath(this));
}

void BookmarksExportDialog::exportBookmarks()
{
    Q_ASSERT(m_currentExporter);

    if (ui->output->text().isEmpty()) {
        return;
    }

    bool ok = m_currentExporter->exportBookmarks(mApp->bookmarks()->rootItem());

    if (!ok) {
        QMessageBox::critical(this, tr("Error!"), m_currentExporter->errorString());
    }
    else {
        close();
    }
}

void BookmarksExportDialog::init()
{
    m_exporters.append(new HtmlExporter(this));

    foreach (BookmarksExporter* exporter, m_exporters) {
        ui->format->addItem(exporter->name());
    }

    m_currentExporter = m_exporters.at(0);
}
