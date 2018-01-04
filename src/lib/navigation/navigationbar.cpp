/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018 David Rosca <nowrep@gmail.com>
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
#include <QWebEngineHistory>
#include <QMouseEvent>
#include <QStyleOption>

static QString titleForUrl(QString title, const QUrl &url)
{
    if (title.isEmpty()) {
        title = url.toString(QUrl::RemoveFragment);
    }
    if (title.isEmpty()) {
        return NavigationBar::tr("Empty Page");
    }
    return QzTools::truncatedText(title, 40);
}

static QIcon iconForPage(const QUrl &url, const QIcon &sIcon)
{
    QIcon icon;
    icon.addPixmap(url.scheme() == QL1S("qupzilla") ? QIcon(QSL(":icons/qupzilla.png")).pixmap(16) : IconProvider::iconForUrl(url).pixmap(16));
    icon.addPixmap(sIcon.pixmap(16), QIcon::Active);
    return icon;
}

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
    QWidget *backNextWidget = new QWidget(this);
    backNextWidget->setLayout(backNextLayout);

    m_reloadStop = new ReloadStopButton(this);

    ToolButton *buttonHome = new ToolButton(this);
    buttonHome->setObjectName("navigation-button-home");
    buttonHome->setToolTip(tr("Home"));
    buttonHome->setToolButtonStyle(Qt::ToolButtonIconOnly);
    buttonHome->setToolbarButtonLook(true);
    buttonHome->setAutoRaise(true);
    buttonHome->setFocusPolicy(Qt::NoFocus);

    ToolButton *buttonAddTab = new ToolButton(this);
    buttonAddTab->setObjectName("navigation-button-addtab");
    buttonAddTab->setToolTip(tr("New Tab"));
    buttonAddTab->setToolButtonStyle(Qt::ToolButtonIconOnly);
    buttonAddTab->setToolbarButtonLook(true);
    buttonAddTab->setAutoRaise(true);
    buttonAddTab->setFocusPolicy(Qt::NoFocus);

    m_menuBack = new Menu(this);
    m_menuBack->setCloseOnMiddleClick(true);
    m_buttonBack->setMenu(m_menuBack);
    connect(m_buttonBack, SIGNAL(aboutToShowMenu()), this, SLOT(aboutToShowHistoryBackMenu()));

    m_menuForward = new Menu(this);
    m_menuForward->setCloseOnMiddleClick(true);
    m_buttonForward->setMenu(m_menuForward);
    connect(m_buttonForward, SIGNAL(aboutToShowMenu()), this, SLOT(aboutToShowHistoryNextMenu()));

    ToolButton *buttonTools = new ToolButton(this);
    buttonTools->setObjectName("navigation-button-tools");
    buttonTools->setPopupMode(QToolButton::InstantPopup);
    buttonTools->setToolbarButtonLook(true);
    buttonTools->setToolTip(tr("Tools"));
    buttonTools->setAutoRaise(true);
    buttonTools->setFocusPolicy(Qt::NoFocus);
    buttonTools->setShowMenuInside(true);

    m_menuTools = new Menu(this);
    buttonTools->setMenu(m_menuTools);
    connect(buttonTools, &ToolButton::aboutToShowMenu, this, &NavigationBar::aboutToShowToolsMenu);

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

    addWidget(backNextWidget, QSL("button-backforward"));
    addWidget(m_reloadStop, QSL("button-reloadstop"));
    addWidget(buttonHome, QSL("button-home"));
    addWidget(buttonAddTab, QSL("button-addtab"));
    addWidget(m_navigationSplitter, QSL("locationbar"));
    addWidget(buttonTools, QSL("button-tools"));

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
    connect(buttonHome, SIGNAL(clicked()), m_window, SLOT(goHome()));
    connect(buttonHome, SIGNAL(middleMouseClicked()), m_window, SLOT(goHomeInNewTab()));
    connect(buttonHome, SIGNAL(controlClicked()), m_window, SLOT(goHomeInNewTab()));
    connect(buttonAddTab, SIGNAL(clicked()), m_window, SLOT(addTab()));
    connect(buttonAddTab, SIGNAL(middleMouseClicked()), m_window->tabWidget(), SLOT(addTabFromClipboard()));
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

void NavigationBar::addWidget(QWidget *widget, const QString &id)
{
    m_widgets[id] = widget;
    reloadLayout();
}

void NavigationBar::removeWidget(const QString &id)
{
    m_widgets.remove(id);
    reloadLayout();
}

void NavigationBar::aboutToShowHistoryBackMenu()
{
    if (!m_menuBack || !m_window->weView()) {
        return;
    }
    m_menuBack->clear();
    QWebEngineHistory* history = m_window->weView()->history();

    int curindex = history->currentItemIndex();
    int count = 0;

    for (int i = curindex - 1; i >= 0; i--) {
        QWebEngineHistoryItem item = history->itemAt(i);
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

    QWebEngineHistory* history = m_window->weView()->history();
    int curindex = history->currentItemIndex();
    int count = 0;

    for (int i = curindex + 1; i < history->count(); i++) {
        QWebEngineHistoryItem item = history->itemAt(i);
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

void NavigationBar::aboutToShowToolsMenu()
{
    m_menuTools->clear();

    m_window->createToolbarsMenu(m_menuTools->addMenu(tr("Toolbars")));
    m_window->createSidebarsMenu(m_menuTools->addMenu(tr("Sidebars")));
    m_menuTools->addSeparator();

    m_menuTools->addAction(IconProvider::settingsIcon(), tr("Configure Toolbar"), this, SLOT(openConfigurationDialog()));
}

void NavigationBar::clearHistory()
{
    QWebEngineHistory* history = m_window->weView()->page()->history();
    history->clear();
    refreshHistory();
}

void NavigationBar::contextMenuRequested(const QPoint &pos)
{
    QMenu menu;
    m_window->createToolbarsMenu(&menu);
    menu.exec(mapToGlobal(pos));
}

void NavigationBar::openConfigurationDialog()
{
}

void NavigationBar::reloadLayout()
{
    const QStringList defaultIds = {
        QSL("button-backforward"),
        QSL("button-reloadstop"),
        QSL("button-home"),
        QSL("locationbar"),
        QSL("button-tools")
    };

    QStringList ids = Settings().value(QSL("NavigationBar/Layout"), defaultIds).toStringList();
    ids.removeDuplicates();
    if (!ids.contains(QSL("locationbar"))) {
        ids.append(QSL("locationbar"));
    }

    while (m_layout->count() != 0) {
        QLayoutItem *item = m_layout->takeAt(0);
        if (!item) {
            continue;
        }
        QWidget *widget = item->widget();
        if (!widget) {
            continue;
        }
        widget->setParent(nullptr);
    }

    for (QWidget *widget : m_widgets) {
        widget->hide();
    }

    for (const QString &id : qAsConst(ids)) {
        QWidget *widget = m_widgets.value(id);
        if (widget) {
            m_layout->addWidget(widget);
            widget->show();
        }
    }

    m_layout->addWidget(m_supMenu);
}

void NavigationBar::loadHistoryIndex()
{
    QWebEngineHistory* history = m_window->weView()->page()->history();

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

    QWebEngineHistory* history = m_window->weView()->page()->history();
    loadHistoryItemInNewTab(history->itemAt(index));
}

void NavigationBar::refreshHistory()
{
    if (mApp->isClosing() || !m_window->weView()) {
        return;
    }

    QWebEngineHistory* history = m_window->weView()->page()->history();
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
    QWebEngineHistory* history = m_window->weView()->page()->history();
    history->back();
}

void NavigationBar::goBackInNewTab()
{
    QWebEngineHistory* history = m_window->weView()->page()->history();

    if (!history->canGoBack()) {
        return;
    }

    loadHistoryItemInNewTab(history->backItem());
}

void NavigationBar::goForward()
{
    QWebEngineHistory* history = m_window->weView()->page()->history();
    history->forward();
}

void NavigationBar::goForwardInNewTab()
{
    QWebEngineHistory* history = m_window->weView()->page()->history();

    if (!history->canGoForward()) {
        return;
    }

    loadHistoryItemInNewTab(history->forwardItem());
}

void NavigationBar::loadHistoryItem(const QWebEngineHistoryItem &item)
{
    m_window->weView()->page()->history()->goToItem(item);

    refreshHistory();
}

void NavigationBar::loadHistoryItemInNewTab(const QWebEngineHistoryItem &item)
{
    TabWidget* tabWidget = m_window->tabWidget();
    int tabIndex = tabWidget->duplicateTab(tabWidget->currentIndex());

    QWebEngineHistory* history = m_window->weView(tabIndex)->page()->history();
    history->goToItem(item);

    if (qzSettings->newTabPosition == Qz::NT_SelectedTab) {
        tabWidget->setCurrentIndex(tabIndex);
    }

}
