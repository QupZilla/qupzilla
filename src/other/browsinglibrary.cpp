/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  nowrep
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
#include "browsinglibrary.h"
#include "ui_browsinglibrary.h"
#include "historymanager.h"
#include "bookmarksmanager.h"
#include "rssmanager.h"
#include "mainapplication.h"
#include "downloaditem.h"
#include "globalfunctions.h"

BrowsingLibrary::BrowsingLibrary(QupZilla* mainClass, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BrowsingLibrary)
    , m_historyManager(new HistoryManager(mainClass))
    , m_bookmarksManager(new BookmarksManager(mainClass))
    , m_rssManager(mApp->rssManager())
    , m_historyLoaded(false)
    , m_bookmarksLoaded(false)
    , m_rssLoaded(false)
{
    ui->setupUi(this);
    QSettings settings(mApp->getActiveProfilPath()+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("BrowsingLibrary");
    resize(settings.value("size", QSize(760, 470)).toSize());
    settings.endGroup();

    qz_centerWidgetOnScreen(this);

    ui->tabs->AddTab(m_historyManager, QIcon(":/icons/other/bighistory.png"), tr("History"));
    ui->tabs->AddTab(m_bookmarksManager, QIcon(":/icons/other/bigstar.png"), tr("Bookmarks"));
    ui->tabs->AddTab(m_rssManager, QIcon(":/icons/other/bigrss.png"), tr("RSS"));

    ui->tabs->SetMode(FancyTabWidget::Mode_LargeSidebar);
    ui->tabs->setFocus();

    connect(ui->tabs, SIGNAL(CurrentChanged(int)), this, SLOT(currentIndexChanged(int)));
    connect(ui->searchLine, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(search()));
}

void BrowsingLibrary::currentIndexChanged(int index)
{
    switch (index) {
    case 0:
        if (!m_historyLoaded) {
            m_historyManager->refreshTable();
            m_historyLoaded = true;
        }
        ui->searchLine->show();
        search();
        break;

    case 1:
        if (!m_bookmarksLoaded) {
            m_bookmarksManager->refreshTable();
            m_bookmarksLoaded = true;
        }
        ui->searchLine->show();
        search();
        break;

    case 2:
        if (!m_rssLoaded) {
            m_rssManager->refreshTable();
            m_rssLoaded = true;
        }
        ui->searchLine->hide();
        break;

    default:
        qWarning("BrowsingLibrary::currentIndexChanged() received index out of range!");
    }
}

void BrowsingLibrary::search()
{
    if (ui->tabs->current_index() == 0)
        m_historyManager->search(ui->searchLine->text());
    else
        m_bookmarksManager->search(ui->searchLine->text());
}

void BrowsingLibrary::showHistory(QupZilla* mainClass)
{
    ui->tabs->SetCurrentIndex(0);
    show();
    m_historyManager->setMainWindow(mainClass);

    if (!m_historyLoaded) {
        m_historyManager->refreshTable();
        m_historyLoaded = true;
    }
    raise();
}

void BrowsingLibrary::showBookmarks(QupZilla* mainClass)
{
    ui->tabs->SetCurrentIndex(1);
    show();
    m_bookmarksManager->setMainWindow(mainClass);

    if (!m_bookmarksLoaded) {
        m_bookmarksManager->refreshTable();
        m_bookmarksLoaded = true;
    }
    raise();
}

void BrowsingLibrary::showRSS(QupZilla* mainClass)
{
    ui->tabs->SetCurrentIndex(2);
    show();
    m_rssManager->setMainWindow(mainClass);
    m_rssManager->refreshTable();
    m_rssLoaded = true;
    raise();
}

void BrowsingLibrary::optimizeDatabase()
{
    mApp->setOverrideCursor(Qt::WaitCursor);
    QString profilePath = mApp->getActiveProfilPath();
    QString sizeBefore = DownloadItem::fileSizeToString(QFileInfo(profilePath+"/browsedata.db").size());
    mApp->history()->optimizeHistory();
    QString sizeAfter = DownloadItem::fileSizeToString(QFileInfo(profilePath+"/browsedata.db").size());
    mApp->restoreOverrideCursor();
    QMessageBox::information(this, tr("Database Optimized"), tr("Database successfuly optimized.<br/><br/><b>Database Size Before: </b>%1<br/><b>Databse Size After: </b>%2").arg(sizeBefore, sizeAfter));
}

void BrowsingLibrary::closeEvent(QCloseEvent *e)
{
    QSettings settings(mApp->getActiveProfilPath()+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("BrowsingLibrary");
    settings.setValue("size", size());
    settings.endGroup();
    e->accept();
}

BrowsingLibrary::~BrowsingLibrary()
{
    delete ui;
}
