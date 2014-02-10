/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
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
#include "bookmarkitem.h"
#include "mainapplication.h"
#include "iconprovider.h"
#include "qztools.h"

#include <QMessageBox>
#include <QFileDialog>

BookmarksImportDialog::BookmarksImportDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::BookmarksImportDialog)
    , m_currentPage(0)
    , m_browser(Firefox)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    ui->browserList->setCurrentRow(0);

#ifndef Q_OS_WIN
    ui->browserList->setItemHidden(ui->browserList->item(IE), true);
#endif

    connect(ui->nextButton, SIGNAL(clicked()), this, SLOT(nextPage()));
    connect(ui->chooseFile, SIGNAL(clicked()), this, SLOT(setFile()));
    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(close()));
}

void BookmarksImportDialog::nextPage()
{
    switch (m_currentPage) {
    case 0:
        if (!ui->browserList->currentItem()) {
            return;
        }

        m_browser = (Browser)(ui->browserList->currentRow());
        setupBrowser(m_browser);
        ui->iconLabel->setPixmap(m_browserPixmap);
        ui->importingFromLabel->setText(tr("<b>Importing from %1</b>").arg(m_browserName));
        ui->fileText1->setText(m_browserFileText);
        ui->fileText2->setText(m_browserFileText2);
        ui->standardDirLabel->setText("<i>" + m_standardDir + "</i>");
        ui->nextButton->setEnabled(false);

        m_currentPage++;
        ui->stackedWidget->setCurrentIndex(m_currentPage);
        break;

    case 1:
        if (ui->fileLine->text().isEmpty()) {
            return;
        }

        if (exportedOK()) {
            m_currentPage++;
            ui->stackedWidget->setCurrentIndex(m_currentPage);
            showExportedBookmarks();
        }
        break;

    default:
        addExportedBookmarks();
        close();
        break;
    }
}

bool BookmarksImportDialog::exportedOK()
{
    if (m_browser == Firefox) {
        FirefoxImporter firefox(this);
        firefox.setFile(ui->fileLine->text());
        if (firefox.openDatabase()) {
            m_exportedFolder = firefox.exportBookmarks();
        }

        if (firefox.error()) {
            QMessageBox::critical(this, tr("Error!"), firefox.errorString());
            return false;
        }
    }
    else if (m_browser == Chrome) {
        ChromeImporter chrome(this);
        chrome.setFile(ui->fileLine->text());
        if (chrome.openFile()) {
            m_exportedFolder = chrome.exportBookmarks();
        }

        if (chrome.error()) {
            QMessageBox::critical(this, tr("Error!"), chrome.errorString());
            return false;
        }
    }
    else if (m_browser == Opera) {
        OperaImporter opera(this);
        opera.setFile(ui->fileLine->text());
        if (opera.openFile()) {
            m_exportedFolder = opera.exportBookmarks();
        }

        if (opera.error()) {
            QMessageBox::critical(this, tr("Error!"), opera.errorString());
            return false;
        }
    }
    else if (m_browser == Html) {
        HtmlImporter html(this);
        html.setFile(ui->fileLine->text());
        if (html.openFile()) {
            m_exportedFolder = html.exportBookmarks();
        }

        if (html.error()) {
            QMessageBox::critical(this, tr("Error!"), html.errorString());
            return false;
        }
    }
#ifdef Q_OS_WIN
    else if (m_browser == IE) {
        IeImporter ie(this);
        ie.setFile(ui->fileLine->text());

        if (ie.openFile()) {
            m_exportedFolder = ie.exportBookmarks();
        }

        if (ie.error()) {
            QMessageBox::critical(this, tr("Error!"), ie.errorString());
        }
    }
#endif

    if (!m_exportedFolder || m_exportedFolder->children().isEmpty()) {
        QMessageBox::critical(this, tr("Error!"), tr("The file doesn't contain any bookmark."));
        return false;
    }

    Q_ASSERT(m_exportedFolder->isFolder());
    return true;
}

void BookmarksImportDialog::showExportedBookmarks()
{
    ui->nextButton->setText(tr("Finish"));

    QTreeWidgetItem* root = new QTreeWidgetItem(ui->treeWidget);
    root->setText(0, m_exportedFolder->title());
    root->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
    ui->treeWidget->addTopLevelItem(root);

    foreach (BookmarkItem* b, m_exportedFolder->children()) {
        // TODO: Multi-level bookmarks
        if (b->isUrl()) {
            QTreeWidgetItem* item = new QTreeWidgetItem(root);
            item->setText(0, b->title());
            item->setIcon(0, _iconForUrl(b->url()));
            item->setText(1, b->urlString());
        }
    }

    ui->treeWidget->expandAll();
}

void BookmarksImportDialog::setFile()
{
#ifdef Q_OS_WIN
    if (m_browser == IE) {
        QString path = QzTools::getExistingDirectory("BookmarksImport-Directory", this, tr("Choose directory..."));
        if (!path.isEmpty()) {
            ui->fileLine->setText(path);
        }
    }
    else
#endif
    {
        QString path = QzTools::getOpenFileName("BookmarksImport-File", this, tr("Choose file..."), QDir::homePath(), m_browserBookmarkFile);
        if (!path.isEmpty()) {
            ui->fileLine->setText(path);
        }
    }

    ui->nextButton->setEnabled(!ui->fileLine->text().isEmpty());
}

void BookmarksImportDialog::addExportedBookmarks()
{
    mApp->bookmarks()->addBookmark(mApp->bookmarks()->unsortedFolder(), m_exportedFolder);
}

void BookmarksImportDialog::setupBrowser(Browser browser)
{
    switch (browser) {
    case Firefox:
        m_browserPixmap = QPixmap(":icons/browsers/firefox.png");
        m_browserName = "Mozilla Firefox";
        m_browserBookmarkFile = "places.sqlite";
        m_browserFileText = tr("Mozilla Firefox stores its bookmarks in <b>places.sqlite</b> SQLite database. "
                               "This file is usually located in ");
        m_browserFileText2 = tr("Please choose this file to begin importing bookmarks.");
        m_standardDir =
#ifdef Q_OS_WIN
            "%APPDATA%/Mozilla/";
#else
            "/home/user/.mozilla/firefox/profilename/";
#endif
        break;

    case Chrome:
        m_browserPixmap = QPixmap(":icons/browsers/chrome.png");
        m_browserName = "Google Chrome";
        m_browserBookmarkFile = "Bookmarks";
        m_browserFileText = tr("Google Chrome stores its bookmarks in <b>Bookmarks</b> text file. "
                               "This file is usually located in ");
        m_browserFileText2 = tr("Please choose this file to begin importing bookmarks.");
        m_standardDir =
#ifdef Q_OS_WIN
            "%APPDATA%/Chrome/Default/";
#else
            "/home/user/.config/chrome/Default/";
#endif

        break;

    case Opera:
        m_browserPixmap = QPixmap(":icons/browsers/opera.png");
        m_browserName = "Opera";
        m_browserBookmarkFile = "bookmarks.adr";
        m_browserFileText = tr("Opera stores its bookmarks in <b>bookmarks.adr</b> text file. "
                               "This file is usually located in ");
        m_browserFileText2 = tr("Please choose this file to begin importing bookmarks.");
        m_standardDir =
#ifdef Q_OS_WIN
            "%APPDATA%/Opera/";
#else
            "/home/user/.opera/";
#endif
        break;

    case Html:
        m_browserPixmap = QPixmap(":icons/browsers/html.png");
        m_browserName = "Html Import";
        m_browserBookmarkFile = "*.htm, *.html";
        m_browserFileText = tr("You can import bookmarks from any browser that supports HTML exporting. "
                               "This file has usually these suffixes");
        m_browserFileText2 = tr("Please choose this file to begin importing bookmarks.");
        m_standardDir = ".html, .htm";
        break;

    case IE:
        m_browserPixmap = QPixmap(":icons/browsers/internet-explorer.png");
        m_browserName = "Internet Explorer";
        m_browserFileText = tr("Internet Explorer stores its bookmarks in <b>Favorites</b> folder. "
                               "This folder is usually located in ");
        m_browserFileText2 = tr("Please choose this folder to begin importing bookmarks.");
        m_standardDir = "C:\\Users\\username\\Favorites\\";
        break;

    default:
        break;
    }
}

BookmarksImportDialog::~BookmarksImportDialog()
{
    delete ui;
}
