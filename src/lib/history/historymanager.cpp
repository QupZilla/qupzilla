/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2017 David Rosca <nowrep@gmail.com>
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
#include "historymanager.h"
#include "ui_historymanager.h"
#include "browserwindow.h"
#include "mainapplication.h"
#include "history.h"
#include "tabwidget.h"
#include "tabbedwebview.h"
#include "headerview.h"
#include "qzsettings.h"
#include "iconprovider.h"

#include <QMessageBox>
#include <QClipboard>

HistoryManager::HistoryManager(BrowserWindow* window, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::HistoryManager)
    , m_window(window)
{
    ui->setupUi(this);
    ui->historyTree->setViewType(HistoryTreeView::HistoryManagerViewType);

    connect(ui->historyTree, SIGNAL(urlActivated(QUrl)), this, SLOT(urlActivated(QUrl)));
    connect(ui->historyTree, SIGNAL(urlCtrlActivated(QUrl)), this, SLOT(urlCtrlActivated(QUrl)));
    connect(ui->historyTree, SIGNAL(urlShiftActivated(QUrl)), this, SLOT(urlShiftActivated(QUrl)));
    connect(ui->historyTree, SIGNAL(contextMenuRequested(QPoint)), this, SLOT(createContextMenu(QPoint)));

    connect(ui->deleteB, SIGNAL(clicked()), ui->historyTree, SLOT(removeSelectedItems()));
    connect(ui->clearAll, SIGNAL(clicked()), this, SLOT(clearHistory()));

    ui->historyTree->setFocus();
}

BrowserWindow* HistoryManager::getWindow()
{
    if (!m_window)
        m_window = mApp->getWindow();
    return m_window.data();
}

void HistoryManager::setMainWindow(BrowserWindow* window)
{
    if (window) {
        m_window = window;
    }
}

void HistoryManager::restoreState(const QByteArray &state)
{
    ui->historyTree->header()->restoreState(state);
}

QByteArray HistoryManager::saveState()
{
    return ui->historyTree->header()->saveState();
}

void HistoryManager::clearHistory()
{
    QMessageBox::StandardButton button = QMessageBox::warning(this, tr("Confirmation"),
                                         tr("Are you sure you want to delete all history?"), QMessageBox::Yes | QMessageBox::No);
    if (button != QMessageBox::Yes) {
        return;
    }

    mApp->history()->clearHistory();
}

void HistoryManager::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Delete:
        ui->historyTree->removeSelectedItems();
        break;
    }

    QWidget::keyPressEvent(event);
}

void HistoryManager::search(const QString &searchText)
{
    ui->historyTree->search(searchText);
}

void HistoryManager::urlActivated(const QUrl &url)
{
    openUrl(url);
}

void HistoryManager::urlCtrlActivated(const QUrl &url)
{
    openUrlInNewTab(url);
}

void HistoryManager::urlShiftActivated(const QUrl &url)
{
    openUrlInNewWindow(url);
}

void HistoryManager::openUrl(const QUrl &url)
{
    const QUrl u = !url.isEmpty() ? url : ui->historyTree->selectedUrl();
    m_window->weView()->load(u);
}

void HistoryManager::openUrlInNewTab(const QUrl &url)
{
    const QUrl u = !url.isEmpty() ? url : ui->historyTree->selectedUrl();
    m_window->tabWidget()->addView(u, qzSettings->newTabPosition);
}

void HistoryManager::openUrlInNewWindow(const QUrl &url)
{
    const QUrl u = !url.isEmpty() ? url : ui->historyTree->selectedUrl();
    mApp->createWindow(Qz::BW_NewWindow, u);
}

void HistoryManager::openUrlInNewPrivateWindow(const QUrl &url)
{
    const QUrl u = !url.isEmpty() ? url : ui->historyTree->selectedUrl();
    mApp->startPrivateBrowsing(u);
}

void HistoryManager::createContextMenu(const QPoint &pos)
{
    QMenu menu;
    QAction* actNewTab = menu.addAction(IconProvider::newTabIcon(), tr("Open in new tab"));
    QAction* actNewWindow = menu.addAction(IconProvider::newWindowIcon(), tr("Open in new window"));
    QAction* actNewPrivateWindow = menu.addAction(IconProvider::privateBrowsingIcon(), tr("Open in new private window"));

    menu.addSeparator();
    QAction* actCopyUrl = menu.addAction(tr("Copy url"), this, SLOT(copyUrl()));
    QAction* actCopyTitle = menu.addAction(tr("Copy title"), this, SLOT(copyTitle()));

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
        actCopyTitle->setDisabled(true);
        actCopyUrl->setDisabled(true);
    }

    menu.exec(pos);
}

void HistoryManager::copyUrl()
{
    QApplication::clipboard()->setText(ui->historyTree->selectedUrl().toString());
}

void HistoryManager::copyTitle()
{
    QApplication::clipboard()->setText(ui->historyTree->currentIndex().data().toString());
}

HistoryManager::~HistoryManager()
{
    delete ui;
}
