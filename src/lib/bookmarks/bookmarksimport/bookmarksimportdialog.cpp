/* ============================================================
* QupZilla - Qt web browser
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
#include "bookmarksimportdialog.h"
#include "ui_bookmarksimportdialog.h"
#include "firefoximporter.h"
#include "chromeimporter.h"
#include "operaimporter.h"
#include "htmlimporter.h"
#include "ieimporter.h"
#include "bookmarks.h"
#include "bookmarkitem.h"
#include "bookmarksmodel.h"
#include "bookmarksitemdelegate.h"
#include "mainapplication.h"

#include <QMessageBox>

BookmarksImportDialog::BookmarksImportDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::BookmarksImportDialog)
    , m_currentPage(0)
    , m_importer(0)
    , m_importedFolder(0)
    , m_model(0)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    ui->browserList->setCurrentRow(0);
    ui->treeView->setItemDelegate(new BookmarksItemDelegate(ui->treeView));

    connect(ui->nextButton, SIGNAL(clicked()), this, SLOT(nextPage()));
    connect(ui->backButton, SIGNAL(clicked()), this, SLOT(previousPage()));
    connect(ui->chooseFile, SIGNAL(clicked()), this, SLOT(setFile()));
    connect(ui->cancelButton, SIGNAL(rejected()), this, SLOT(close()));

#ifndef Q_OS_WIN
    ui->browserList->setItemHidden(ui->browserList->item(IE), true);
#endif
}

BookmarksImportDialog::~BookmarksImportDialog()
{
    ui->treeView->setModel(0);
    delete m_model;
    delete m_importedFolder;
    delete m_importer;
    delete ui;
}

void BookmarksImportDialog::nextPage()
{
    switch (m_currentPage) {
    case 0:
        if (!ui->browserList->currentItem()) {
            return;
        }

        switch (ui->browserList->currentRow()) {
        case Firefox:
            m_importer = new FirefoxImporter;
            break;
        case Chrome:
            m_importer = new ChromeImporter;
            break;
        case Opera:
            m_importer = new OperaImporter;
            break;
        case IE:
            m_importer = new IeImporter;
            break;
        case Html:
            m_importer = new HtmlImporter;
            break;
        default:
            Q_ASSERT(!"Unreachable");
            break;
        }

        ui->fileLine->clear();
        showImporterPage();

        ui->nextButton->setEnabled(false);
        ui->backButton->setEnabled(true);
        ui->stackedWidget->setCurrentIndex(++m_currentPage);
        break;

    case 1:
        if (ui->fileLine->text().isEmpty()) {
            return;
        }

        if (m_importer->prepareImport()) {
            m_importedFolder = m_importer->importBookmarks();
        }

        if (m_importer->error()) {
            QMessageBox::critical(this, tr("Error!"), m_importer->errorString());
            return;
        }

        if (!m_importedFolder || m_importedFolder->children().isEmpty()) {
            QMessageBox::warning(this, tr("Error!"), tr("No bookmarks were found."));
            return;
        }

        Q_ASSERT(m_importedFolder->isFolder());

        ui->stackedWidget->setCurrentIndex(++m_currentPage);
        ui->nextButton->setText(tr("Finish"));
        showExportedBookmarks();
        break;

    case 2:
        addExportedBookmarks();
        close();
        break;

    default:
        Q_ASSERT(!"Unreachable");
    }
}

void BookmarksImportDialog::previousPage()
{
    switch (m_currentPage) {
    case 0:
        break;

    case 1:
        ui->nextButton->setEnabled(true);
        ui->backButton->setEnabled(false);
        ui->stackedWidget->setCurrentIndex(--m_currentPage);

        delete m_importer;
        m_importer = 0;
        break;

    case 2:
        showImporterPage();

        ui->nextButton->setText(tr("Next >"));
        ui->nextButton->setEnabled(true);
        ui->backButton->setEnabled(true);
        ui->stackedWidget->setCurrentIndex(--m_currentPage);

        ui->treeView->setModel(0);
        delete m_model;
        m_model = 0;

        delete m_importedFolder;
        m_importedFolder = 0;
        break;

    default:
        Q_ASSERT(!"Unreachable");
    }
}

void BookmarksImportDialog::setFile()
{
    Q_ASSERT(m_importer);

    ui->fileLine->setText(m_importer->getPath(this));
    ui->nextButton->setEnabled(!ui->fileLine->text().isEmpty());
}

void BookmarksImportDialog::showImporterPage()
{
    ui->iconLabel->setPixmap(ui->browserList->currentItem()->icon().pixmap(48));
    ui->importingFromLabel->setText(tr("<b>Importing from %1</b>").arg(ui->browserList->currentItem()->text()));
    ui->fileText1->setText(m_importer->description());
    ui->standardDirLabel->setText(QString("<i>%1</i>").arg(m_importer->standardPath()));
}

void BookmarksImportDialog::showExportedBookmarks()
{
    m_model = new BookmarksModel(m_importedFolder, 0, this);
    ui->treeView->setModel(m_model);
    ui->treeView->header()->resizeSection(0, ui->treeView->header()->width() / 2);
    ui->treeView->expandAll();
}

void BookmarksImportDialog::addExportedBookmarks()
{
    mApp->bookmarks()->addBookmark(mApp->bookmarks()->unsortedFolder(), m_importedFolder);
    m_importedFolder = 0;
}
