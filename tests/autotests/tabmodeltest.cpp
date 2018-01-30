/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2018 David Rosca <nowrep@gmail.com>
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
#include "tabmodeltest.h"
#include "tabmodel.h"
#include "webtab.h"
#include "tabwidget.h"
#include "tabbedwebview.h"
#include "mainapplication.h"
#include "browserwindow.h"

#include "modeltest.h"

#include <QtTest/QtTest>

void TabModelTest::initTestCase()
{
}

void TabModelTest::cleanupTestCase()
{
}

void TabModelTest::basicTest()
{
    BrowserWindow *w = mApp->createWindow(Qz::BW_NewWindow);

    TabModel model1(w);
    ModelTest modelTest(&model1);

    QSignalSpy rowsInsertedSpy(&model1, &TabModel::rowsInserted);
    QSignalSpy rowsRemovedSpy(&model1, &TabModel::rowsRemoved);

    QCOMPARE(model1.rowCount(), 0);

    rowsInsertedSpy.wait();

    QCOMPARE(rowsInsertedSpy.count(), 1);
    WebTab *tab0 = w->weView(0)->webTab();
    QCOMPARE(rowsInsertedSpy.at(0).at(0).value<QModelIndex>(), QModelIndex());
    QCOMPARE(rowsInsertedSpy.at(0).at(1).toInt(), 0);
    QCOMPARE(rowsInsertedSpy.at(0).at(2).toInt(), 0);
    QCOMPARE(model1.data(model1.index(0, 0), TabModel::WebTabRole).value<WebTab*>(), tab0);

    rowsInsertedSpy.clear();

    w->tabWidget()->addView(QUrl("http://test.com"));

    QCOMPARE(rowsInsertedSpy.count(), 1);
    WebTab *tab1 = w->weView(1)->webTab();
    QCOMPARE(rowsInsertedSpy.at(0).at(0).value<QModelIndex>(), QModelIndex());
    QCOMPARE(rowsInsertedSpy.at(0).at(1).toInt(), 1);
    QCOMPARE(rowsInsertedSpy.at(0).at(2).toInt(), 1);
    QCOMPARE(model1.data(model1.index(1, 0), TabModel::WebTabRole).value<WebTab*>(), tab1);

    w->tabWidget()->moveTab(0, 1);
    QCOMPARE(w->weView(0)->webTab(), tab1);
    QCOMPARE(w->weView(1)->webTab(), tab0);

    w->tabWidget()->moveTab(1, 0);
    QCOMPARE(w->weView(0)->webTab(), tab0);
    QCOMPARE(w->weView(1)->webTab(), tab1);

    w->tabWidget()->moveTab(0, 1);
    QCOMPARE(w->weView(0)->webTab(), tab1);
    QCOMPARE(w->weView(1)->webTab(), tab0);

    QCOMPARE(rowsRemovedSpy.count(), 0);

    w->tabWidget()->closeTab(1);

    QCOMPARE(rowsRemovedSpy.count(), 1);
    QCOMPARE(rowsRemovedSpy.at(0).at(0).value<QModelIndex>(), QModelIndex());
    QCOMPARE(rowsRemovedSpy.at(0).at(1).toInt(), 1);
    QCOMPARE(rowsRemovedSpy.at(0).at(2).toInt(), 1);

    QCOMPARE(model1.rowCount(), 1);

    TabModel model2(w);
    ModelTest modelTest2(&model2);
    QCOMPARE(model2.rowCount(), 1);

    QTest::qWait(1);
    delete w;
}

void TabModelTest::dataTest()
{
    BrowserWindow *w = mApp->createWindow(Qz::BW_NewWindow);
    TabModel model(w);
    ModelTest modelTest(&model);

    QTRY_COMPARE(model.rowCount(), 1);

    WebTab *tab0 = w->weView(0)->webTab();
    QCOMPARE(model.index(0, 0).data(TabModel::TitleRole).toString(), tab0->title());
    QCOMPARE(model.index(0, 0).data(Qt::DisplayRole).toString(), tab0->title());
    QCOMPARE(model.index(0, 0).data(TabModel::IconRole).value<QIcon>().pixmap(16), tab0->icon().pixmap(16));
    QCOMPARE(model.index(0, 0).data(Qt::DecorationRole).value<QIcon>().pixmap(16), tab0->icon().pixmap(16));
    QCOMPARE(model.index(0, 0).data(TabModel::PinnedRole).toBool(), tab0->isPinned());
    QCOMPARE(model.index(0, 0).data(TabModel::RestoredRole).toBool(), tab0->isRestored());

    w->tabWidget()->addView(QUrl("http://test.com"));

    WebTab *tab1 = w->weView(1)->webTab();

    QSignalSpy dataChangedSpy(&model, &TabModel::dataChanged);

    tab1->setParentTab(tab0);

    QCOMPARE(dataChangedSpy.count(), 2);
    QCOMPARE(dataChangedSpy.at(0).at(0).value<QModelIndex>(), model.index(0, 0));
    QCOMPARE(dataChangedSpy.at(0).at(1).value<QModelIndex>(), model.index(0, 0));
    QCOMPARE(dataChangedSpy.at(0).at(2).value<QVector<int>>(), QVector<int>{TabModel::ChildTabsRole});
    QCOMPARE(model.index(0, 0).data(TabModel::ChildTabsRole).value<QVector<WebTab*>>(), QVector<WebTab*>{tab1});

    QCOMPARE(dataChangedSpy.at(1).at(0).value<QModelIndex>(), model.index(1, 0));
    QCOMPARE(dataChangedSpy.at(1).at(1).value<QModelIndex>(), model.index(1, 0));
    QCOMPARE(dataChangedSpy.at(1).at(2).value<QVector<int>>(), QVector<int>{TabModel::ParentTabRole});
    QCOMPARE(model.index(1, 0).data(TabModel::ParentTabRole).value<WebTab*>(), tab0);

    QTest::qWait(1);
    delete w;
}
