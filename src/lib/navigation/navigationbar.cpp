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
#include "navigationbar.h"
#include "toolbutton.h"
#include "browserwindow.h"
#include "mainapplication.h"
#include "iconprovider.h"
#include "websearchbar.h"
#include "reloadstopbutton.h"
#include "enhancedmenu.h"
#include "tabwidget.h"
#include "tabbedwebview.h"
#include "webpage.h"
#include "qzsettings.h"
#include "qztools.h"

#include <QTimer>
#include <QSplitter>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QWebHistory>
#include <QMouseEvent>
#include <QStyleOption>

NavigationBar::NavigationBar(BrowserWindow* window)
    : QWidget(window)
    , m_window(window)
{
    setObjectName(QSL("navigationbar"));

    m_layout = new QHBoxLayout(this);
    m_layout->setMargin(style()->pixelMetric(QStyle::PM_ToolBarItemMargin, 0, this));
    m_layout->setSpacing(style()->pixelMetric(QStyle::PM_ToolBarItemSpacing, 0, this));
    setLayout(m_layout);

    m_buttonBack = new ToolButton(this);
    m_buttonBack->setObjectName("navigation-button-back");
    m_buttonBack->setToolTip(tr("Back"));
    m_buttonBack->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_buttonBack->setToolbarButtonLook(true);
    m_buttonBack->setAutoRaise(true);
    m_buttonBack->setEnabled(false);
    m_buttonBack->setFocusPolicy(Qt::NoFocus);

    m_buttonForward = new ToolButton(this);
    m_buttonForward->setObjectName("navigation-button-next");
    m_buttonForward->setToolTip(tr("Forward"));
    m_buttonForward->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_buttonForward->setToolbarButtonLook(true);
    m_buttonForward->setAutoRaise(true);
    m_buttonForward->setEnabled(false);
    m_buttonForward->setFocusPolicy(Qt::NoFocus);

    QHBoxLayout* backNextLayout = new QHBoxLayout();
    backNextLayout->setContentsMargins(0, 0, 0, 0);
    backNextLayout->setSpacing(0);
    backNextLayout->addWidget(m_buttonBack);
    backNextLayout->addWidget(m_buttonForward);

    m_reloadStop = new ReloadStopButton(this);

    m_buttonHome = new ToolButton(this);
    m_buttonHome->setObjectName("navigation-button-home");
    m_buttonHome->setToolTip(tr("Home"));
    m_buttonHome->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_buttonHome->setToolbarButtonLook(true);
    m_buttonHome->setAutoRaise(true);
    m_buttonHome->setFocusPolicy(Qt::NoFocus);

    m_buttonAddTab = new ToolButton(this);
    m_buttonAddTab->setObjectName("navigation-button-addtab");
    m_buttonAddTab->setToolTip(tr("New Tab"));
    m_buttonAddTab->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_buttonAddTab->setToolbarButtonLook(true);
    m_buttonAddTab->setAutoRaise(true);
    m_buttonAddTab->setFocusPolicy(Qt::NoFocus);

    m_menuBack = new Menu(this);
    m_menuBack->setCloseOnMiddleClick(true);
    m_buttonBack->setMenu(m_menuBack);
    connect(m_buttonBack, SIGNAL(aboutToShowMenu()), this, SLOT(aboutToShowHistoryBackMenu()));

    m_menuForward = new Menu(this);
    m_menuForward->setCloseOnMiddleClick(true);
    m_buttonForward->setMenu(m_menuForward);
    connect(m_buttonForward, SIGNAL(aboutToShowMenu()), this, SLOT(aboutToShowHistoryNextMenu()));

    m_supMenu = new ToolButton(this);
    m_supMenu->setObjectName("navigation-button-supermenu");
    m_supMenu->setPopupMode(QToolButton::InstantPopup);
    m_supMenu->setToolbarButtonLook(true);
    m_supMenu->setToolTip(tr("Main Menu"));
    m_supMenu->setAutoRaise(true);
    m_supMenu->setFocusPolicy(Qt::NoFocus);
    m_supMenu->setMenu(m_window->superMenu());
    m_supMenu->setShowMenuInside(true);

    m_searchLine = new WebSearchBar(m_window);

    m_navigationSplitter = new QSplitter(this);
    m_navigationSplitter->addWidget(m_window->tabWidget()->locationBars());
    m_navigationSplitter->addWidget(m_searchLine);

    m_navigationSplitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    m_navigationSplitter->setCollapsible(0, false);

    m_exitFullscreen = new ToolButton(this);
    m_exitFullscreen->setObjectName("navigation-button-exitfullscreen");
    m_exitFullscreen->setToolTip(tr("Exit Fullscreen"));
    m_exitFullscreen->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_exitFullscreen->setToolbarButtonLook(true);
    m_exitFullscreen->setFocusPolicy(Qt::NoFocus);
    m_exitFullscreen->setAutoRaise(true);
    m_exitFullscreen->setVisible(false);

    m_layout->addLayout(backNextLayout);
    m_layout->addWidget(m_reloadStop);
    m_layout->addWidget(m_buttonHome);
    m_layout->addWidget(m_buttonAddTab);
    m_layout->addWidget(m_navigationSplitter);
    m_layout->addWidget(m_supMenu);
    m_layout->addWidget(m_exitFullscreen);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequested(QPoint)));

    connect(m_buttonBack, SIGNAL(clicked()), this, SLOT(goBack()));
    connect(m_buttonBack, SIGNAL(middleMouseClicked()), this, SLOT(goBackInNewTab()));
    connect(m_buttonBack, SIGNAL(controlClicked()), this, SLOT(goBackInNewTab()));
    connect(m_buttonForward, SIGNAL(clicked()), this, SLOT(goForward()));
    connect(m_buttonForward, SIGNAL(middleMouseClicked()), this, SLOT(goForwardInNewTab()));
    connect(m_buttonForward, SIGNAL(controlClicked()), this, SLOT(goForwardInNewTab()));

    connect(m_reloadStop, SIGNAL(stopClicked()), this, SLOT(stop()));
    connect(m_reloadStop, SIGNAL(reloadClicked()), this, SLOT(reload()));
    connect(m_buttonHome, SIGNAL(clicked()), m_window, SLOT(goHome()));
    connect(m_buttonHome, SIGNAL(middleMouseClicked()), m_window, SLOT(goHomeInNewTab()));
    connect(m_buttonHome, SIGNAL(controlClicked()), m_window, SLOT(goHomeInNewTab()));
    connect(m_buttonAddTab, SIGNAL(clicked()), m_window, SLOT(addTab()));
    connect(m_buttonAddTab, SIGNAL(middleMouseClicked()), m_window->tabWidget(), SLOT(addTabFromClipboard()));
    connect(m_exitFullscreen, SIGNAL(clicked(bool)), m_window, SLOT(toggleFullScreen()));
}

void NavigationBar::setSplitterSizes(int locationBar, int websearchBar)
{
    QList<int> sizes;

    if (locationBar == 0) {
        int splitterWidth = m_navigationSplitter->width();
        sizes << (int)((double)splitterWidth * .80) << (int)((double)splitterWidth * .20);
    }
    else {
        sizes << locationBar << websearchBar;
    }

    m_navigationSplitter->setSizes(sizes);
}

void NavigationBar::showReloadButton()
{
    m_reloadStop->showReloadButton();
}

void NavigationBar::showStopButton()
{
    m_reloadStop->showStopButton();
}

void NavigationBar::setSuperMenuVisible(bool visible)
{
    m_supMenu->setVisible(visible);
}

int NavigationBar::layoutMargin() const
{
    return m_layout->margin();
}

void NavigationBar::setLayoutMargin(int margin)
{
    m_layout->setMargin(margin);
}

int NavigationBar::layoutSpacing() const
{
    return m_layout->spacing();
}

void NavigationBar::setLayoutSpacing(int spacing)
{
    m_layout->setSpacing(spacing);
}

void NavigationBar::aboutToShowHistoryBackMenu()
{
    if (!m_menuBack || !m_window->weView()) {
        return;
    }
    m_menuBack->clear();
    QWebHistory* history = m_window->weView()->history();

    int curindex = history->currentItemIndex();
    int count = 0;

    for (int i = curindex - 1; i >= 0; i--) {
        QWebHistoryItem item = history->itemAt(i);
        if (item.isValid()) {
            QString title = titleForUrl(item.title(), item.url());

            const QIcon icon = iconForPage(item.url(), IconProvider::standardIcon(QStyle::SP_ArrowBack));
            Action* act = new Action(icon, title);
            act->setData(i);
            connect(act, SIGNAL(triggered()), this, SLOT(loadHistoryIndex()));
            connect(act, SIGNAL(ctrlTriggered()), this, SLOT(loadHistoryIndexInNewTab()));
            m_menuBack->addAction(act);
        }

        count++;
        if (count == 20) {
            break;
        }
    }

    m_menuBack->addSeparator();
    m_menuBack->addAction(tr("Clear history"), this, SLOT(clearHistory()));
}

void NavigationBar::aboutToShowHistoryNextMenu()
{
    if (!m_menuForward || !m_window->weView()) {
        return;
    }
    m_menuForward->clear();

    QWebHistory* history = m_window->weView()->history();
    int curindex = history->currentItemIndex();
    int count = 0;

    for (int i = curindex + 1; i < history->count(); i++) {
        QWebHistoryItem item = history->itemAt(i);
        if (item.isValid()) {
            QString title = titleForUrl(item.title(), item.url());

            const QIcon icon = iconForPage(item.url(), IconProvider::standardIcon(QStyle::SP_ArrowForward));
            Action* act = new Action(icon, title);
            act->setData(i);
            connect(act, SIGNAL(triggered()), this, SLOT(loadHistoryIndex()));
            connect(act, SIGNAL(ctrlTriggered()), this, SLOT(loadHistoryIndexInNewTab()));
            m_menuForward->addAction(act);
        }

        count++;
        if (count == 20) {
            break;
        }
    }

    m_menuForward->addSeparator();
    m_menuForward->addAction(tr("Clear history"), this, SLOT(clearHistory()));
}

void NavigationBar::clearHistory()
{
    QWebHistory* history = m_window->weView()->page()->history();
    history->clear();
    refreshHistory();
}

void NavigationBar::contextMenuRequested(const QPoint &pos)
{
    QMenu menu;
    m_window->createToolbarsMenu(&menu);
    menu.exec(mapToGlobal(pos));
}

void NavigationBar::loadHistoryIndex()
{
    QWebHistory* history = m_window->weView()->page()->history();

    if (QAction* action = qobject_cast<QAction*>(sender())) {
        loadHistoryItem(history->itemAt(action->data().toInt()));
    }
}

void NavigationBar::loadHistoryIndexInNewTab(int index)
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        index = action->data().toInt();
    }

    if (index == -1) {
        return;
    }

    QWebHistory* history = m_window->weView()->page()->history();
    loadHistoryItemInNewTab(history->itemAt(index));
}

void NavigationBar::refreshHistory()
{
    if (mApp->isClosing() || !m_window->weView()) {
        return;
    }

    QWebHistory* history = m_window->weView()->page()->history();
    m_buttonBack->setEnabled(history->canGoBack());
    m_buttonForward->setEnabled(history->canGoForward());
}

void NavigationBar::stop()
{
    m_window->action(QSL("View/Stop"))->trigger();
}

void NavigationBar::reload()
{
    m_window->action(QSL("View/Reload"))->trigger();
}

void NavigationBar::goBack()
{
    QWebHistory* history = m_window->weView()->page()->history();
    history->back();
}

void NavigationBar::goBackInNewTab()
{
    QWebHistory* history = m_window->weView()->page()->history();

    if (!history->canGoBack()) {
        return;
    }

    loadHistoryItemInNewTab(history->backItem());
}

void NavigationBar::goForward()
{
    QWebHistory* history = m_window->weView()->page()->history();
    history->forward();
}

void NavigationBar::goForwardInNewTab()
{
    QWebHistory* history = m_window->weView()->page()->history();

    if (!history->canGoForward()) {
        return;
    }

    loadHistoryItemInNewTab(history->forwardItem());
}

QString NavigationBar::titleForUrl(QString title, const QUrl &url)
{
    if (title.isEmpty()) {
        title = url.toString(QUrl::RemoveFragment);
    }

    if (title.isEmpty()) {
        return tr("Empty Page");
    }

    return QzTools::truncatedText(title, 40);
}

QIcon NavigationBar::iconForPage(const QUrl &url, const QIcon &sIcon)
{
    QIcon icon;
    icon.addPixmap(url.scheme() == QL1S("qupzilla") ? QIcon(QSL(":icons/qupzilla.png")).pixmap(16, 16) : IconProvider::iconForUrl(url).pixmap(16, 16));
    icon.addPixmap(sIcon.pixmap(16, 16), QIcon::Active);
    return icon;
}

void NavigationBar::loadHistoryItem(const QWebHistoryItem &item)
{
    m_window->weView()->page()->history()->goToItem(item);

    refreshHistory();
}

void NavigationBar::loadHistoryItemInNewTab(const QWebHistoryItem &item)
{
    TabWidget* tabWidget = m_window->tabWidget();
    int tabIndex = tabWidget->duplicateTab(tabWidget->currentIndex());

    QWebHistory* history = m_window->weView(tabIndex)->page()->history();
    history->goToItem(item);

    if (qzSettings->newTabPosition == Qz::NT_SelectedTab) {
        tabWidget->setCurrentIndex(tabIndex);
    }

}
