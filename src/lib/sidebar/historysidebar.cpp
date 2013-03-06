/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2013  David Rosca <nowrep@gmail.com>
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
#include "qupzilla.h"
#include "tabwidget.h"
#include "tabbedwebview.h"
#include "mainapplication.h"
#include "historymodel.h"
#include "qzsettings.h"

HistorySideBar::HistorySideBar(QupZilla* mainClass, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::HistorySideBar)
    , p_QupZilla(mainClass)
{
    ui->setupUi(this);

    ui->historyTree->setColumnHidden(1, true);
    ui->historyTree->setColumnHidden(2, true);
    ui->historyTree->setColumnHidden(3, true);
    ui->historyTree->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(ui->historyTree, SIGNAL(openLink(QUrl,HistoryView::OpenBehavior)), this, SLOT(openLink(QUrl,HistoryView::OpenBehavior)));
    connect(ui->search, SIGNAL(textEdited(QString)), ui->historyTree->filterModel(), SLOT(setFilterFixedString(QString)));
}

void HistorySideBar::openLink(const QUrl &url, HistoryView::OpenBehavior openIn)
{
    if (openIn == HistoryView::OpenInNewTab) {
        p_QupZilla->tabWidget()->addView(url, qzSettings->newTabPosition);
    }
    else {
        p_QupZilla->weView()->load(url);
    }
}

HistorySideBar::~HistorySideBar()
{
    delete ui;
}
