/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2014-2018 David Rosca <nowrep@gmail.com>
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
#include "historymenu.h"
#include "iconprovider.h"
#include "browserwindow.h"
#include "tabbedwebview.h"
#include "mainapplication.h"
#include "closedtabsmanager.h"
#include "tabwidget.h"
#include "qztools.h"
#include "history.h"
#include "qzsettings.h"
#include "sqldatabase.h"
#include "closedwindowsmanager.h"

#include <QApplication>
#include <QWebEngineHistory>

HistoryMenu::HistoryMenu(QWidget* parent)
    : Menu(parent)
{
    init();
}

void HistoryMenu::setMainWindow(BrowserWindow* window)
{
    m_window = window;
}

void HistoryMenu::goBack()
{
    if (m_window) {
        m_window->goBack();
    }
}

void HistoryMenu::goForward()
{
    if (m_window) {
        m_window->goForward();
    }
}

void HistoryMenu::goHome()
{
    if (m_window) {
        m_window->goHome();
    }
}

void HistoryMenu::showHistoryManager()
{
    if (m_window) {
        m_window->showHistoryManager();
    }
}

void HistoryMenu::aboutToShow()
{
    // Set enabled states for Back/Forward actions according to current WebView
    TabbedWebView* view = m_window ? m_window->weView() : 0;

    if (view) {
        actions().at(0)->setEnabled(view->history()->canGoBack());
        actions().at(1)->setEnabled(view->history()->canGoForward());
    }

    while (actions().count() != 8) {
        QAction* act = actions().at(8);
        if (act->menu()) {
            act->menu()->clear();
        }
        removeAction(act);
        delete act;
    }

    addSeparator();

    QSqlQuery query(SqlDatabase::instance()->database());
    query.exec(QSL("SELECT title, url FROM history ORDER BY date DESC LIMIT 10"));

    while (query.next()) {
        const QUrl url = query.value(1).toUrl();
        const QString title = QzTools::truncatedText(query.value(0).toString(), 40);

        Action* act = new Action(title);
        act->setData(url);
        act->setIcon(IconProvider::iconForUrl(url));
        connect(act, SIGNAL(triggered()), this, SLOT(historyEntryActivated()));
        connect(act, SIGNAL(ctrlTriggered()), this, SLOT(historyEntryCtrlActivated()));
        connect(act, SIGNAL(shiftTriggered()), this, SLOT(historyEntryShiftActivated()));
        addAction(act);
    }
}

void HistoryMenu::aboutToHide()
{
    // Enable Back/Forward actions to ensure shortcuts are working
    actions().at(0)->setEnabled(true);
    actions().at(1)->setEnabled(true);
}

void HistoryMenu::aboutToShowMostVisited()
{
    m_menuMostVisited->clear();

    const QVector<HistoryEntry> mostVisited = mApp->history()->mostVisited(10);

    foreach (const HistoryEntry &entry, mostVisited) {
        Action* act = new Action(QzTools::truncatedText(entry.title, 40));
        act->setData(entry.url);
        act->setIcon(IconProvider::iconForUrl(entry.url));
        connect(act, SIGNAL(triggered()), this, SLOT(historyEntryActivated()));
        connect(act, SIGNAL(ctrlTriggered()), this, SLOT(historyEntryCtrlActivated()));
        connect(act, SIGNAL(shiftTriggered()), this, SLOT(historyEntryShiftActivated()));
        m_menuMostVisited->addAction(act);
    }

    if (m_menuMostVisited->isEmpty()) {
        m_menuMostVisited->addAction(tr("Empty"))->setEnabled(false);
    }
}

void HistoryMenu::aboutToShowClosedTabs()
{
    m_menuClosedTabs->clear();

    if (!m_window) {
        return;
    }

    TabWidget* tabWidget = m_window->tabWidget();

    const auto closedTabs = tabWidget->closedTabsManager()->closedTabs();
    for (int i = 0; i < closedTabs.count(); ++i) {
        const ClosedTabsManager::Tab tab = closedTabs.at(i);
        const QString title = QzTools::truncatedText(tab.tabState.title, 40);
        m_menuClosedTabs->addAction(tab.tabState.icon, title, tabWidget, SLOT(restoreClosedTab()))->setData(i);
    }

    if (m_menuClosedTabs->isEmpty()) {
        m_menuClosedTabs->addAction(tr("Empty"))->setEnabled(false);
    }
    else {
        m_menuClosedTabs->addSeparator();
        m_menuClosedTabs->addAction(tr("Restore All Closed Tabs"), tabWidget, SLOT(restoreAllClosedTabs()));
        m_menuClosedTabs->addAction(tr("Clear list"), tabWidget, SLOT(clearClosedTabsList()));
    }
}

void HistoryMenu::aboutToShowClosedWindows()
{
    m_menuClosedWindows->clear();

    ClosedWindowsManager *manager = mApp->closedWindowsManager();

    const auto closedWindows = manager->closedWindows();
    for (int i = 0; i < closedWindows.count(); ++i) {
        const ClosedWindowsManager::Window window = closedWindows.at(i);
        const QString title = QzTools::truncatedText(window.title, 40);
        QAction *act = m_menuClosedWindows->addAction(window.icon, title, manager, SLOT(restoreClosedWindow()));
        if (i == 0) {
            act->setShortcut(QKeySequence(QSL("Ctrl+Shift+N")));
            act->setShortcutContext(Qt::WidgetShortcut);
        } else {
            act->setData(i);
        }
    }

    if (m_menuClosedWindows->isEmpty()) {
        m_menuClosedWindows->addAction(tr("Empty"))->setEnabled(false);
    } else {
        m_menuClosedWindows->addSeparator();
        m_menuClosedWindows->addAction(tr("Restore All Closed Windows"), manager, SLOT(restoreAllClosedWindows()));
        m_menuClosedWindows->addAction(tr("Clear list"), manager, SLOT(clearClosedWindows()));
    }
}

void HistoryMenu::historyEntryActivated()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        openUrl(action->data().toUrl());
    }
}

void HistoryMenu::historyEntryCtrlActivated()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        openUrlInNewTab(action->data().toUrl());
    }
}

void HistoryMenu::historyEntryShiftActivated()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        openUrlInNewWindow(action->data().toUrl());
    }
}

void HistoryMenu::openUrl(const QUrl &url)
{
    if (m_window) {
        m_window->loadAddress(url);
    }
}

void HistoryMenu::openUrlInNewTab(const QUrl &url)
{
    if (m_window) {
        m_window->tabWidget()->addView(url, qzSettings->newTabPosition);
    }
}

void HistoryMenu::openUrlInNewWindow(const QUrl &url)
{
    mApp->createWindow(Qz::BW_NewWindow, url);
}

void HistoryMenu::init()
{
    setTitle(tr("Hi&story"));

    QAction* act = addAction(IconProvider::standardIcon(QStyle::SP_ArrowBack), tr("&Back"), this, SLOT(goBack()));
    act->setShortcut(QzTools::actionShortcut(QKeySequence::Back, Qt::ALT + Qt::Key_Left, QKeySequence::Forward, Qt::ALT + Qt::Key_Right));

    act = addAction(IconProvider::standardIcon(QStyle::SP_ArrowForward), tr("&Forward"), this, SLOT(goForward()));
    act->setShortcut(QzTools::actionShortcut(QKeySequence::Forward, Qt::ALT + Qt::Key_Right, QKeySequence::Back, Qt::ALT + Qt::Key_Left));

    act = addAction(QIcon::fromTheme("go-home"), tr("&Home"), this, SLOT(goHome()));
    act->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Home));

    act = addAction(QIcon::fromTheme("deep-history", QIcon(":/icons/menu/history.svg")), tr("Show &All History"), this, SLOT(showHistoryManager()));
    act->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_H));

    addSeparator();

    connect(this, SIGNAL(aboutToShow()), this, SLOT(aboutToShow()));
    connect(this, SIGNAL(aboutToHide()), this, SLOT(aboutToHide()));

    m_menuMostVisited = new Menu(tr("Most Visited"), this);
    connect(m_menuMostVisited, SIGNAL(aboutToShow()), this, SLOT(aboutToShowMostVisited()));

    m_menuClosedTabs = new Menu(tr("Closed Tabs"));
    connect(m_menuClosedTabs, SIGNAL(aboutToShow()), this, SLOT(aboutToShowClosedTabs()));

    m_menuClosedWindows = new Menu(tr("Closed Windows"));
    connect(m_menuClosedWindows, &QMenu::aboutToShow, this, &HistoryMenu::aboutToShowClosedWindows);

    addMenu(m_menuMostVisited);
    addMenu(m_menuClosedTabs);
    addMenu(m_menuClosedWindows);
}
