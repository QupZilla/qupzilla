/* ============================================================
* TabManager plugin for QupZilla
* Copyright (C) 2013-2018 S. Razi Alavizadeh <s.r.alavizadeh@gmail.com>
* Copyright (C) 2017-2018 David Rosca <nowrep@gmail.com>
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
#include "tabmanagerwidgetcontroller.h"
#include "tabmanagerwidget.h"
#include "abstractbuttoninterface.h"
#include "browserwindow.h"
#include "tabwidget.h"
#include "mainapplication.h"
#include "tabbar.h"
#include "statusbar.h"
#include "navigationbar.h"

#include <QDesktopWidget>
#include <QAction>
#include <QStyle>

#include <QDebug>

class TabManagerButton : public AbstractButtonInterface
{
public:
    explicit TabManagerButton(QObject *parent = nullptr)
        : AbstractButtonInterface(parent)
    {
    }

    QString id() const override
    {
        return QSL("tabmanager-icon");
    }

    QString name() const override
    {
        return tr("Tab Manager button");
    }
};


TabManagerWidgetController::TabManagerWidgetController(QObject* parent)
    : SideBarInterface(parent)
    , m_defaultTabManager(0)
    , m_groupType(TabManagerWidget::GroupByWindow)
{
}

TabManagerWidgetController::~TabManagerWidgetController()
{
}

QString TabManagerWidgetController::title() const
{
    return tr("Tab Manager");
}

QAction* TabManagerWidgetController::createMenuAction()
{
    QAction* act = new QAction(tr("Tab Manager"), this);
    act->setCheckable(true);
    act->setIcon(QIcon(":tabmanager/data/tabmanager.png"));
    act->setShortcut(QKeySequence("Ctrl+Shift+M"));
    act->setData("TabManager");

    return act;
}

QWidget* TabManagerWidgetController::createSideBarWidget(BrowserWindow* mainWindow)
{
    return createTabManagerWidget(mainWindow, mainWindow);
}

AbstractButtonInterface* TabManagerWidgetController::createStatusBarIcon(BrowserWindow* mainWindow)
{
    if (!defaultTabManager()) {
        return 0;
    }

    if (m_statusBarIcons.contains(mainWindow)) {
        return m_statusBarIcons.value(mainWindow);
    }

    TabManagerButton* icon = new TabManagerButton(this);
    icon->setIcon(QPixmap(":tabmanager/data/tabmanager.png"));
    icon->setTitle(tr("Tab Manager"));
    icon->setToolTip(tr("Show Tab Manager"));
    connect(icon, &AbstractButtonInterface::clicked, this, [=](AbstractButtonInterface::ClickController *c) {
        if (!defaultTabManager()) {
            return;
        }

        static int frameWidth = (defaultTabManager()->frameGeometry().width() - defaultTabManager()->geometry().width()) / 2;
        static int titleBarHeight = defaultTabManager()->style()->pixelMetric(QStyle::PM_TitleBarHeight);

        QSize newSize(defaultTabManager()->width(), mainWindow->height() - titleBarHeight - frameWidth);
        QRect newGeo(c->popupPosition(newSize), newSize);
        defaultTabManager()->setGeometry(newGeo);
        raiseTabManager();

        QTimer::singleShot(0, this, [=]() {
            c->popupClosed();
        });
    });

    QAction* showAction = createMenuAction();
    showAction->setCheckable(false);
    showAction->setParent(icon);
    mainWindow->addAction(showAction);
    connect(showAction, SIGNAL(triggered()), this, SLOT(raiseTabManager()));

    m_statusBarIcons.insert(mainWindow, icon);
    m_actions.insert(mainWindow, showAction);

    return icon;
}

TabManagerWidget::GroupType TabManagerWidgetController::groupType()
{
    return m_groupType;
}

void TabManagerWidgetController::setGroupType(TabManagerWidget::GroupType type)
{
    m_groupType = type;
}

TabManagerWidget* TabManagerWidgetController::createTabManagerWidget(BrowserWindow* mainClass, QWidget* parent, bool defaultWidget)
{
    TabManagerWidget* tabManagerWidget = new TabManagerWidget(mainClass, parent, defaultWidget);
    tabManagerWidget->setGroupType(m_groupType);

    if (defaultWidget) {
        m_defaultTabManager = tabManagerWidget;
        QAction* showAction = createMenuAction();
        showAction->setCheckable(false);
        showAction->setParent(m_defaultTabManager);
        m_defaultTabManager->addAction(showAction);
        connect(showAction, SIGNAL(triggered()), this, SLOT(raiseTabManager()));
        connect(tabManagerWidget, SIGNAL(showSideBySide()), this, SLOT(showSideBySide()));
    }
    else {
        m_defaultTabManager = 0;
    }

    connect(tabManagerWidget, SIGNAL(groupTypeChanged(TabManagerWidget::GroupType)), this, SLOT(setGroupType(TabManagerWidget::GroupType)));
    connect(this, SIGNAL(requestRefreshTree(WebPage*)), tabManagerWidget, SLOT(delayedRefreshTree(WebPage*)));

    emit requestRefreshTree();

    return tabManagerWidget;
}

TabManagerWidget* TabManagerWidgetController::defaultTabManager()
{
    return m_defaultTabManager;
}

void TabManagerWidgetController::addStatusBarIcon(BrowserWindow* window)
{
    if (window) {
        window->statusBar()->addButton(createStatusBarIcon(window));
        window->navigationBar()->addToolButton(createStatusBarIcon(window));
    }
}

void TabManagerWidgetController::removeStatusBarIcon(BrowserWindow* window)
{
    if (window) {
        window->statusBar()->removeButton(m_statusBarIcons.value(window));
        window->navigationBar()->removeToolButton(m_statusBarIcons.value(window));
        window->removeAction(m_actions.value(window));
        delete m_actions.value(window);
        delete m_statusBarIcons.value(window);
        m_statusBarIcons.remove(window);
        m_actions.remove(window);
    }
}

void TabManagerWidgetController::mainWindowDeleted(BrowserWindow* window)
{
    removeStatusBarIcon(window);

    emit requestRefreshTree();
}

void TabManagerWidgetController::raiseTabManager()
{
    if (!defaultTabManager()) {
        return;
    }

    defaultTabManager()->activateWindow();
    defaultTabManager()->showNormal();
    defaultTabManager()->raise();
}

void TabManagerWidgetController::showSideBySide()
{
    if (!defaultTabManager()) {
        return;
    }
    const QRect &availableGeometry = mApp->desktop()->availableGeometry(defaultTabManager());
    static int frameWidth = (defaultTabManager()->frameGeometry().width() - defaultTabManager()->geometry().width()) / 2;
    static int titleBarHeight = defaultTabManager()->style()->pixelMetric(QStyle::PM_TitleBarHeight);

    QRect managerRect(availableGeometry.left() + frameWidth, availableGeometry.top() + titleBarHeight,
                      defaultTabManager()->width(), availableGeometry.height() - titleBarHeight - frameWidth);
    QRect qupzillaRect(managerRect.topRight().x() + 2 * frameWidth, managerRect.top(),
                       availableGeometry.width() - managerRect.width() - 4 * frameWidth, managerRect.height());

    defaultTabManager()->setGeometry(managerRect);
    mApp->getWindow()->setGeometry(qupzillaRect);
    mApp->getWindow()->showNormal();
    mApp->getWindow()->raise();

    defaultTabManager()->show();
    defaultTabManager()->activateWindow();
    defaultTabManager()->raise();
}

void TabManagerWidgetController::emitRefreshTree()
{
    emit requestRefreshTree();
}
