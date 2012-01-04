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
#include "bookmarksimportdialog.h"
#include "ui_bookmarksimportdialog.h"
#include "firefoximporter.h"
#include "chromeimporter.h"
#include "operaimporter.h"
#include "htmlimporter.h"
#include "mainapplication.h"
#include "iconfetcher.h"
#include "networkmanager.h"

BookmarksImportDialog::BookmarksImportDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::BookmarksImportDialog)
    , m_currentPage(0)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    ui->browserList->setCurrentRow(0);

    connect(ui->nextButton, SIGNAL(clicked()), this, SLOT(nextPage()));
    connect(ui->chooseFile, SIGNAL(clicked()), this, SLOT(setFile()));
    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->stopButton, SIGNAL(clicked()), this, SLOT(stopDownloading()));

    HtmlImporter html(this);
    html.setFile("/home/david/bookmarks.html");
    html.exportBookmarks();
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
            startFetchingIcons();
        }
        break;

    case 2:
        addExportedBookmarks();
        close();
        break;

    default:
        break;
    }
}

void BookmarksImportDialog::startFetchingIcons()
{
    ui->nextButton->setText(tr("Finish"));
    ui->nextButton->setEnabled(false);
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(m_exportedBookmarks.count());

    int i = 0;
    foreach(BookmarksModel::Bookmark b, m_exportedBookmarks) {
        QTreeWidgetItem* item = new QTreeWidgetItem();
        item->setText(0, b.title);
        item->setIcon(0, QWebSettings::globalSettings()->webGraphic(QWebSettings::DefaultFrameIconGraphic));
        item->setText(1, b.url.toString());
        item->setWhatsThis(0, QString::number(i));

        ui->treeWidget->addTopLevelItem(item);
        i++;

        IconFetcher* fetcher = new IconFetcher(this);
        fetcher->setNetworkAccessManager(mApp->networkManager());
        connect(fetcher, SIGNAL(finished()), this, SLOT(loadFinished()));
        connect(fetcher, SIGNAL(iconFetched(QIcon)), this, SLOT(iconFetched(QIcon)));
        fetcher->fetchIcon(b.url);

        QPair<IconFetcher*, QUrl> pair;
        pair.first = fetcher;
        pair.second = b.url;
        m_fetchers.append(pair);
    }
}

void BookmarksImportDialog::stopDownloading()
{
    ui->nextButton->setEnabled(true);
    ui->stopButton->hide();
    ui->progressBar->setValue(ui->progressBar->maximum());
    ui->fetchingLabel->setText(tr("Please press Finish to complete importing process."));
}

void BookmarksImportDialog::loadFinished()
{
    ui->progressBar->setValue(ui->progressBar->value() + 1);

    if (ui->progressBar->value() == ui->progressBar->maximum()) {
        ui->stopButton->hide();
        ui->nextButton->setEnabled(true);
        ui->fetchingLabel->setText(tr("Please press Finish to complete importing process."));
    }
}

void BookmarksImportDialog::iconFetched(const QIcon &icon)
{
    IconFetcher* fetcher = qobject_cast<IconFetcher*>(sender());
    if (!fetcher) {
        return;
    }

    QUrl url;
    for (int i = 0; i < m_fetchers.count(); i++) {
        QPair<IconFetcher*, QUrl> pair = m_fetchers.at(i);
        if (pair.first == fetcher) {
            url = pair.second;
            break;
        }
    }

    if (url.isEmpty()) {
        return;
    }

    QList<QTreeWidgetItem*> items = ui->treeWidget->findItems(url.toString(), Qt::MatchExactly, 1);
    if (items.count() == 0) {
        return;
    }

    foreach(QTreeWidgetItem * item, items) {
        item->setIcon(0, icon);

        foreach(BookmarksModel::Bookmark b, m_exportedBookmarks) {
            if (b.url == url) {
                m_exportedBookmarks.removeOne(b);
                b.icon = icon;
                m_exportedBookmarks.append(b);
                break;
            }
        }
    }
}

bool BookmarksImportDialog::exportedOK()
{
    if (m_browser == Firefox) {
        FirefoxImporter firefox(this);
        firefox.setFile(ui->fileLine->text());
        if (firefox.openDatabase()) {
            m_exportedBookmarks = firefox.exportBookmarks();
        }

        if (firefox.error()) {
            QMessageBox::critical(this, tr("Error!"), firefox.errorString());
            return false;
        }
        return true;
    }
    else if (m_browser == Chrome) {
        ChromeImporter chrome(this);
        chrome.setFile(ui->fileLine->text());
        if (chrome.openFile()) {
            m_exportedBookmarks = chrome.exportBookmarks();
        }

        if (chrome.error()) {
            QMessageBox::critical(this, tr("Error!"), chrome.errorString());
            return false;
        }
        return true;
    }
    else if (m_browser == Opera) {
        OperaImporter opera(this);
        opera.setFile(ui->fileLine->text());
        if (opera.openFile()) {
            m_exportedBookmarks = opera.exportBookmarks();
        }

        if (opera.error()) {
            QMessageBox::critical(this, tr("Error!"), opera.errorString());
            return false;
        }
        return true;
    }
    else if (m_browser == Html) {
        HtmlImporter html(this);
        html.setFile(ui->fileLine->text());
        if (html.openFile()) {
            m_exportedBookmarks = html.exportBookmarks();
        }

        if (html.error()) {
            QMessageBox::critical(this, tr("Error!"), html.errorString());
            return false;
        }
        return true;
    }

    return false;
}

void BookmarksImportDialog::setFile()
{
#ifdef Q_WS_WIN
    if (m_browser == IE) {
        QString path = QFileDialog::getExistingDirectory(this, tr("Choose directory..."));
        if (!path.isEmpty()) {
            ui->fileLine->setText(path);
        }
    }
    else
#endif
    {
        QString path = QFileDialog::getOpenFileName(this, tr("Choose file..."), QDir::homePath(), m_browserBookmarkFile);
        if (!path.isEmpty()) {
            ui->fileLine->setText(path);
        }
    }

    ui->nextButton->setEnabled(!ui->fileLine->text().isEmpty());
}

void BookmarksImportDialog::addExportedBookmarks()
{
    qApp->setOverrideCursor(Qt::WaitCursor);

    BookmarksModel* model = mApp->bookmarksModel();

    if (m_exportedBookmarks.count() > 0) {
        model->createFolder(m_exportedBookmarks.at(0).folder);
    }

    foreach(BookmarksModel::Bookmark b, m_exportedBookmarks)
    model->saveBookmark(b.url, b.title, b.icon, b.folder);

    qApp->restoreOverrideCursor();
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
#ifdef Q_WS_WIN
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
#ifdef Q_WS_WIN
            "%APPDATA%/Opera/";
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
#ifdef Q_WS_WIN
            "%APPDATA%/Chrome/Default/";
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
        m_browserPixmap = QPixmap(":icons/browsers/ie.png");
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
    if (m_fetchers.count() > 0) {
        for (int i = 0; i < m_fetchers.count(); i++) {
            tr("");
            IconFetcher* fetcher = m_fetchers.at(i).first;
            fetcher->deleteLater();
        }
    }

    delete ui;
}
