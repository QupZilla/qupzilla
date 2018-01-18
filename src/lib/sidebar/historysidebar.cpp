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
#include "historysidebar.h"
#include "ui_historysidebar.h"
#include "browserwindow.h"
#include "tabwidget.h"
#include "tabbedwebview.h"
#include "mainapplication.h"
#include "qzsettings.h"
#include "iconprovider.h"

HistorySideBar::HistorySideBar(BrowserWindow* window, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::HistorySideBar)
    , m_window(window)
{
    ui->setupUi(this);
    ui->historyTree->setViewType(HistoryTreeView::HistorySidebarViewType);

    connect(ui->historyTree, SIGNAL(urlActivated(QUrl)), this, SLOT(urlActivated(QUrl)));
    connect(ui->historyTree, SIGNAL(urlCtrlActivated(QUrl)), this, SLOT(urlCtrlActivated(QUrl)));
    connect(ui->historyTree, SIGNAL(urlShiftActivated(QUrl)), this, SLOT(urlShiftActivated(QUrl)));
    connect(ui->historyTree, SIGNAL(contextMenuRequested(QPoint)), this, SLOT(createContextMenu(QPoint)));

    connect(ui->search, SIGNAL(textEdited(QString)), ui->historyTree, SLOT(search(QString)));
}

void HistorySideBar::urlActivated(const QUrl &url)
{
    openUrl(url);
}

void HistorySideBar::urlCtrlActivated(const QUrl &url)
{
    openUrlInNewTab(url);
}

void HistorySideBar::urlShiftActivated(const QUrl &url)
{
    openUrlInNewWindow(url);
}

void HistorySideBar::openUrl(const QUrl &url)
{
    const QUrl u = !url.isEmpty() ? url : ui->historyTree->selectedUrl();
    m_window->weView()->load(u);
}

void HistorySideBar::openUrlInNewTab(const QUrl &url)
{
    const QUrl u = !url.isEmpty() ? url : ui->historyTree->selectedUrl();
    m_window->tabWidget()->addView(u, qzSettings->newTabPosition);
}

void HistorySideBar::openUrlInNewWindow(const QUrl &url)
{
    const QUrl u = !url.isEmpty() ? url : ui->historyTree->selectedUrl();
    mApp->createWindow(Qz::BW_NewWindow, u);
}

void HistorySideBar::openUrlInNewPrivateWindow(const QUrl &url)
{
    const QUrl u = !url.isEmpty() ? url : ui->historyTree->selectedUrl();
    mApp->startPrivateBrowsing(u);
}

void HistorySideBar::createContextMenu(const QPoint &pos)
{
    QMenu menu;
    QAction* actNewTab = menu.addAction(IconProvider::newTabIcon(), tr("Open in new tab"));
    QAction* actNewWindow = menu.addAction(IconProvider::newWindowIcon(), tr("Open in new window"));
    QAction* actNewPrivateWindow = menu.addAction(IconProvider::privateBrowsingIcon(), tr("Open in new private window"));

    menu.addSeparator();
    QAction* actDelete = menu.addAction(QIcon::fromTheme(QSL("edit-delete")), tr("Delete"));

    connect(actNewTab, SIGNAL(triggered()), this, SLOT(openUrlInNewTab()));
    connect(actNewWindow, SIGNAL(triggered()), this, SLOT(openUrlInNewWindow()));
    connect(actNewPrivateWindow, SIGNAL(triggered()), this, SLOT(openUrlInNewPrivateWindow()));
    connect(actDelete, SIGNAL(triggered()), ui->historyTree, SLOT(removeSelectedItems()));

    if (ui->historyTree->selectedUrl().isEmpty()) {
        actNewTab->setDisabled(true);
        actNewWindow->setDisabled(true);
        actNewPrivateWindow->setDisabled(true);
    }

    menu.exec(pos);
}

HistorySideBar::~HistorySideBar()
{
    delete ui;
}
