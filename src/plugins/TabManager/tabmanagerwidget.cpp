/* ============================================================
* TabManager plugin for QupZilla
* Copyright (C) 2013-2017  S. Razi Alavizadeh <s.r.alavizadeh@gmail.com>
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
#include "tabmanagerwidget.h"
#include "ui_tabmanagerwidget.h"
#include "mainapplication.h"
#include "browserwindow.h"
#include "webtab.h"
#include "webpage.h"
#include "tabbedwebview.h"
#include "tabwidget.h"
#include "locationbar.h"
#include "bookmarkstools.h"
#include "bookmarkitem.h"
#include "bookmarks.h"
#include "tabmanagerplugin.h"
#include "tldextractor/tldextractor.h"
#include "tabmanagerdelegate.h"


#include <QDesktopWidget>
#include <QDialogButtonBox>
#include <QStackedWidget>
#include <QDialog>
#include <QTimer>
#include <QLabel>


TLDExtractor* TabManagerWidget::s_tldExtractor = 0;

TabManagerWidget::TabManagerWidget(BrowserWindow* mainClass, QWidget* parent, bool defaultWidget)
    : QWidget(parent)
    , ui(new Ui::TabManagerWidget)
    , p_QupZilla(mainClass)
    , m_webPage(0)
    , m_isRefreshing(false)
    , m_refreshBlocked(false)
    , m_waitForRefresh(false)
    , m_isDefaultWidget(defaultWidget)
{
    if(s_tldExtractor == 0)
    {
        s_tldExtractor = TLDExtractor::instance();
        s_tldExtractor->setDataSearchPaths(QStringList() << TabManagerPlugin::settingsPath());
    }

    ui->setupUi(this);
    ui->treeWidget->setUniformRowHeights(true);
    ui->treeWidget->setColumnCount(2);
    ui->treeWidget->header()->hide();
    ui->treeWidget->header()->setStretchLastSection(false);
    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    ui->treeWidget->header()->resizeSection(1, 16);

    ui->treeWidget->setExpandsOnDoubleClick(false);
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->treeWidget->installEventFilter(this);
    ui->filterBar->installEventFilter(this);

    QPushButton* closeButton = new QPushButton(ui->filterBar);
    closeButton->setFlat(true);
    closeButton->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
    ui->filterBar->addWidget(closeButton, LineEdit::RightSide);
    ui->filterBar->hide();

    ui->treeWidget->setItemDelegate(new TabManagerDelegate(ui->treeWidget));

    connect(closeButton, SIGNAL(clicked(bool)), this, SLOT(filterBarClosed()));
    connect(ui->filterBar, SIGNAL(textChanged(QString)), this, SLOT(filterChanged(QString)));
    connect(ui->treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(onItemActivated(QTreeWidgetItem*,int)));
    connect(ui->treeWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customContextMenuRequested(QPoint)));
}

TabManagerWidget::~TabManagerWidget()
{
    delete ui;
}

void TabManagerWidget::setGroupType(GroupType type)
{
    m_groupType = type;
}

QString TabManagerWidget::domainFromUrl(const QUrl &url, bool useHostName)
{
    QString appendString = QL1S(":");
    QString urlString = url.toString();

    if (url.scheme() == "file") {
        return tr("Local File System:");
    }
    else if (url.scheme() == "qupzilla" || urlString.isEmpty()) {
        return tr("QupZilla:");
    }
    else if (url.scheme() == "ftp") {
        appendString.prepend(tr(" [FTP]"));
    }

    QString host = url.host();
    if (host.isEmpty()) {
        return urlString.append(appendString);
    }

    if (useHostName || host.contains(QRegExp("^[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+$"))) {
        if (host.startsWith("www.", Qt::CaseInsensitive)) {
            host.remove(0, 4);
        }

        return host.append(appendString);
    }
    else {
        const QString registeredDomain = s_tldExtractor->registrableDomain(host);

        if (!registeredDomain.isEmpty()) {
            host = registeredDomain;
        }

        return host.append(appendString);
    }
}

void TabManagerWidget::delayedRefreshTree(WebPage* p)
{
    if (m_refreshBlocked || m_waitForRefresh) {
        return;
    }

    if (m_isRefreshing && !p) {
        return;
    }

    m_webPage = p;
    m_waitForRefresh = true;
    QTimer::singleShot(50, this, SLOT(refreshTree()));
}

void TabManagerWidget::refreshTree()
{
    if (m_refreshBlocked) {
        return;
    }

    if (m_isRefreshing && !m_webPage) {
        return;
    }

    // store selected items
    QList<QWidget*> selectedTabs;
    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem* winItem = ui->treeWidget->topLevelItem(i);
        if (winItem->checkState(0) == Qt::Unchecked) {
            continue;
        }

        for (int j = 0; j < winItem->childCount(); ++j) {
            QTreeWidgetItem* tabItem = winItem->child(j);
            if (tabItem->checkState(0) == Qt::Unchecked) {
                continue;
            }
            selectedTabs << qvariant_cast<QWidget*>(tabItem->data(0, WebTabPointerRole));
        }
    }

    ui->treeWidget->clear();

    if (m_groupType == GroupByHost) {
        groupByDomainName(true);
    }
    else if (m_groupType == GroupByDomain) {
        groupByDomainName();
    }
    else { // fallback to GroupByWindow
        m_groupType = GroupByWindow;
        groupByWindow();
    }

    // restore selected items
    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem* winItem = ui->treeWidget->topLevelItem(i);

        for (int j = 0; j < winItem->childCount(); ++j) {
            QTreeWidgetItem* tabItem = winItem->child(j);

            if (selectedTabs.contains(qvariant_cast<QWidget*>(tabItem->data(0, WebTabPointerRole)))) {
                tabItem->setCheckState(0, Qt::Checked);
            }
        }
    }

    filterChanged(m_filterText, true);
    ui->treeWidget->expandAll();
    m_isRefreshing = false;
    m_waitForRefresh = false;
}

void TabManagerWidget::onItemActivated(QTreeWidgetItem* item, int column)
{
    if (!item) {
        return;
    }

    BrowserWindow* mainWindow = qobject_cast<BrowserWindow*>(qvariant_cast<QWidget*>(item->data(0, QupZillaPointerRole)));
    QWidget* tabWidget = qvariant_cast<QWidget*>(item->data(0, WebTabPointerRole));

    if (!mainWindow) {
        return;
    }

    if (column == 1) {
        if (item->childCount() == 0 && tabWidget)
            mainWindow->tabWidget()->requestCloseTab(mainWindow->tabWidget()->indexOf(tabWidget));
        else if (item->childCount() > 0)
            QMetaObject::invokeMethod(mainWindow, "addTab");
        return;
    }

    if (mainWindow->isMinimized()) {
        mainWindow->showNormal();
    }
    else {
        mainWindow->show();
    }
    mainWindow->activateWindow();
    mainWindow->raise();
    mainWindow->weView()->setFocus();

    if (tabWidget && tabWidget != mainWindow->tabWidget()->currentWidget()) {
        mainWindow->tabWidget()->setCurrentIndex(mainWindow->tabWidget()->indexOf(tabWidget));
    }
}

bool TabManagerWidget::isTabSelected()
{
    bool selected = false;
    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem* parentItem = ui->treeWidget->topLevelItem(i);
        if (parentItem->checkState(0) != Qt::Unchecked) {
            selected = true;
            break;
        }
    }

    return selected;
}

void TabManagerWidget::customContextMenuRequested(const QPoint &pos)
{
    QMenu menu;
    QAction* action;
    QMenu groupTypeSubmenu(tr("Group by"));
    action = groupTypeSubmenu.addAction(tr("&Window"), this, SLOT(changeGroupType()));
    action->setData(GroupByWindow);
    action->setCheckable(true);
    action->setChecked(m_groupType == GroupByWindow);

    action = groupTypeSubmenu.addAction(tr("&Domain"), this, SLOT(changeGroupType()));
    action->setData(GroupByDomain);
    action->setCheckable(true);
    action->setChecked(m_groupType == GroupByDomain);

    action = groupTypeSubmenu.addAction(tr("&Host"), this, SLOT(changeGroupType()));
    action->setData(GroupByHost);
    action->setCheckable(true);
    action->setChecked(m_groupType == GroupByHost);

    menu.addMenu(&groupTypeSubmenu);

    if (m_isDefaultWidget) {
        menu.addAction(QIcon(":/tabmanager/data/side-by-side.png"), tr("&Show side by side"), this, SIGNAL(showSideBySide()))->setObjectName("sideBySide");
    }

    menu.addSeparator();

    if (isTabSelected()) {
        menu.addAction(QIcon(":/tabmanager/data/tab-detach.png"), tr("&Detach checked tabs"), this, SLOT(processActions()))->setObjectName("detachSelection");
        menu.addAction(QIcon(":/tabmanager/data/tab-bookmark.png"), tr("Book&mark checked tabs"), this, SLOT(processActions()))->setObjectName("bookmarkSelection");
        menu.addAction(QIcon(":/tabmanager/data/tab-close.png"), tr("&Close checked tabs"), this, SLOT(processActions()))->setObjectName("closeSelection");
    }

    menu.exec(ui->treeWidget->viewport()->mapToGlobal(pos));
}

void TabManagerWidget::filterChanged(const QString &filter, bool force)
{
    if (force || filter != m_filterText) {
        m_filterText = filter.simplified();
        ui->treeWidget->itemDelegate()->setProperty("filterText", m_filterText);
        if (m_filterText.isEmpty()) {
            for (int i = 0; i < ui->treeWidget->topLevelItemCount(); ++i) {
                QTreeWidgetItem* parentItem = ui->treeWidget->topLevelItem(i);
                for (int j = 0; j < parentItem->childCount(); ++j) {
                    QTreeWidgetItem* childItem = parentItem->child(j);
                    childItem->setHidden(false);
                }
                parentItem->setHidden(false);
                parentItem->setExpanded(true);
            }

            return;
        }

        const QRegularExpression filterRegExp(filter.simplified().replace(QChar(' '), QLatin1String(".*"))
                                              .append(QLatin1String(".*")).prepend(QLatin1String(".*")),
                                              QRegularExpression::CaseInsensitiveOption);

        for (int i = 0; i < ui->treeWidget->topLevelItemCount(); ++i) {
            QTreeWidgetItem* parentItem = ui->treeWidget->topLevelItem(i);
            int visibleChildCount = 0;
            for (int j = 0; j < parentItem->childCount(); ++j) {
                QTreeWidgetItem* childItem = parentItem->child(j);
                if (childItem->text(0).contains(filterRegExp) || childItem->data(0, UrlRole).toString().simplified().contains(filterRegExp)) {
                    ++visibleChildCount;
                    childItem->setHidden(false);
                }
                else {
                    childItem->setHidden(true);
                }
            }

            if (visibleChildCount == 0) {
                parentItem->setHidden(true);
            }
            else {
                parentItem->setHidden(false);
                parentItem->setExpanded(true);
            }
        }
    }
}

void TabManagerWidget::filterBarClosed()
{
    ui->filterBar->clear();
    ui->filterBar->hide();
    ui->treeWidget->setFocusProxy(0);
    ui->treeWidget->setFocus();
}

bool TabManagerWidget::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        const QString text = keyEvent->text().simplified();

        if (obj == ui->treeWidget) {
            // switch to tab/window on enter
            if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
                onItemActivated(ui->treeWidget->currentItem(), 0);
                return QObject::eventFilter(obj, event);
            }

            if (!text.isEmpty() || ((keyEvent->modifiers() & Qt::ControlModifier) && keyEvent->key() == Qt::Key_F)) {
                ui->filterBar->show();
                ui->treeWidget->setFocusProxy(ui->filterBar);
                ui->filterBar->setFocus();
                if (!text.isEmpty() && text.at(0).isPrint()) {
                    ui->filterBar->setText(ui->filterBar->text() + text);
                }

                return true;
            }
        }
        else if (obj == ui->filterBar) {
            bool isNavigationOrActionKey = keyEvent->key() == Qt::Key_Up ||
                    keyEvent->key() == Qt::Key_Down ||
                    keyEvent->key() == Qt::Key_PageDown ||
                    keyEvent->key() == Qt::Key_PageUp ||
                    keyEvent->key() == Qt::Key_Enter ||
                    keyEvent->key() == Qt::Key_Return;

            // send scroll or action press key to treeWidget
            if (isNavigationOrActionKey) {
                QKeyEvent ev(QKeyEvent::KeyPress, keyEvent->key(), keyEvent->modifiers());
                QApplication::sendEvent(ui->treeWidget, &ev);
                return false;
            }
        }
    }

    if (obj == ui->treeWidget && (event->type() == QEvent::Resize || event->type() == QEvent::Show))
        ui->treeWidget->setColumnHidden(1, ui->treeWidget->viewport()->width() < 150);

    return QObject::eventFilter(obj, event);
}

void TabManagerWidget::processActions()
{
    if (!sender()) {
        return;
    }

    m_refreshBlocked = true;

    QHash<BrowserWindow*, WebTab*> selectedTabs;

    const QString &command = sender()->objectName();

    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem* winItem = ui->treeWidget->topLevelItem(i);
        if (winItem->checkState(0) == Qt::Unchecked) {
            continue;
        }

        for (int j = 0; j < winItem->childCount(); ++j) {
            QTreeWidgetItem* tabItem = winItem->child(j);
            if (tabItem->checkState(0) == Qt::Unchecked) {
                continue;
            }

            BrowserWindow* mainWindow = qobject_cast<BrowserWindow*>(qvariant_cast<QWidget*>(tabItem->data(0, QupZillaPointerRole)));
            WebTab* webTab = qobject_cast<WebTab*>(qvariant_cast<QWidget*>(tabItem->data(0, WebTabPointerRole)));

            // current supported actions are not applied to pinned tabs
            if (webTab->isPinned()) {
                tabItem->setCheckState(0, Qt::Unchecked);
                continue;
            }

            if (command == "closeSelection") {
                if (webTab->url().toString() == "qupzilla:restore") {
                    continue;
                }
                selectedTabs.insertMulti(mainWindow, webTab);
            }
            else if (command == "detachSelection" || command == "bookmarkSelection") {
                selectedTabs.insertMulti(mainWindow, webTab);
            }
        }
        winItem->setCheckState(0, Qt::Unchecked);
    }

    if (!selectedTabs.isEmpty()) {
        if (command == "closeSelection") {
            closeSelectedTabs(selectedTabs);
        }
        else if (command == "detachSelection") {
            detachSelectedTabs(selectedTabs);
        }
        else if (command == "bookmarkSelection") {
            bookmarkSelectedTabs(selectedTabs);
        }
    }

    m_refreshBlocked = false;
    delayedRefreshTree();
}

void TabManagerWidget::changeGroupType()
{
    QAction* action = qobject_cast<QAction*>(sender());

    if (action) {
        int type = action->data().toInt();

        if (m_groupType != GroupType(type)) {
            m_groupType = GroupType(type);

            delayedRefreshTree();

            emit groupTypeChanged(m_groupType);
        }
    }
}

void TabManagerWidget::closeSelectedTabs(const QHash<BrowserWindow*, WebTab*> &tabsHash)
{
    if (tabsHash.isEmpty()) {
        return;
    }

    const QList<BrowserWindow*> &windows = tabsHash.uniqueKeys();
    foreach (BrowserWindow* mainWindow, windows) {
        QList<WebTab*> tabs = tabsHash.values(mainWindow);

        foreach (WebTab* webTab, tabs) {
            mainWindow->tabWidget()->requestCloseTab(webTab->tabIndex());
        }
    }
}

void TabManagerWidget::detachSelectedTabs(const QHash<BrowserWindow*, WebTab*> &tabsHash)
{
    // TODO: use TabWidget::detachTab()
    if (tabsHash.isEmpty() ||
            (tabsHash.uniqueKeys().size() == 1 &&
             tabsHash.size() == tabsHash.keys().at(0)->tabWidget()->count())) {
        return;
    }

    BrowserWindow* newWindow = mApp->createWindow(Qz::BW_OtherRestoredWindow);
    newWindow->move(mApp->desktop()->availableGeometry(this).topLeft() + QPoint(30, 30));

    const QList<BrowserWindow*> &windows = tabsHash.uniqueKeys();
    foreach (BrowserWindow* mainWindow, windows) {
        const QList<WebTab*> &tabs = tabsHash.values(mainWindow);
        foreach (WebTab* webTab, tabs) {
            mainWindow->tabWidget()->locationBars()->removeWidget(webTab->locationBar());

            disconnect(webTab->webView(), SIGNAL(wantsCloseTab(int)), mainWindow->tabWidget(), SLOT(closeTab(int)));
            disconnect(webTab->webView(), SIGNAL(changed()), mainWindow->tabWidget(), SIGNAL(changed()));
            disconnect(webTab->webView(), SIGNAL(ipChanged(QString)), mainWindow->ipLabel(), SLOT(setText(QString)));

            webTab->detach();
            if (mainWindow && mainWindow->tabWidget()->count() == 0) {
                mainWindow->close();
                mainWindow = 0;
            }

            newWindow->tabWidget()->addView(webTab);
        }
    }
}

bool TabManagerWidget::bookmarkSelectedTabs(const QHash<BrowserWindow*, WebTab*> &tabsHash)
{
    QDialog* dialog = new QDialog(getQupZilla(), Qt::WindowStaysOnTopHint | Qt::MSWindowsFixedSizeDialogHint);
    QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom, dialog);
    QLabel* label = new QLabel(dialog);
    BookmarksFoldersButton* folderButton = new BookmarksFoldersButton(dialog);

    QDialogButtonBox* box = new QDialogButtonBox(dialog);
    box->addButton(QDialogButtonBox::Ok);
    box->addButton(QDialogButtonBox::Cancel);
    QObject::connect(box, SIGNAL(rejected()), dialog, SLOT(reject()));
    QObject::connect(box, SIGNAL(accepted()), dialog, SLOT(accept()));

    layout->addWidget(label);
    layout->addWidget(folderButton);
    layout->addWidget(box);

    label->setText(tr("Choose folder for bookmarks:"));
    dialog->setWindowTitle(tr("Bookmark Selected Tabs"));

    QSize size = dialog->size();
    size.setWidth(350);
    dialog->resize(size);
    dialog->exec();

    if (dialog->result() == QDialog::Rejected) {
        return false;
    }

    foreach (WebTab* tab, tabsHash) {
        if (!tab->url().isEmpty()) {
            BookmarkItem* bookmark = new BookmarkItem(BookmarkItem::Url);
            bookmark->setTitle(tab->title());
            bookmark->setUrl(tab->url());
            mApp->bookmarks()->addBookmark(folderButton->selectedFolder(), bookmark);
        }
    }

    delete dialog;
    return true;
}

QTreeWidgetItem* TabManagerWidget::createEmptyItem(QTreeWidgetItem* parent, bool addToTree)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(addToTree ? (parent ? parent : ui->treeWidget->invisibleRootItem()) : 0);
    item->setFlags(item->flags() | (parent ? Qt::ItemIsUserCheckable : Qt::ItemIsUserCheckable | Qt::ItemIsTristate));
    item->setCheckState(0, Qt::Unchecked);

    return item;
}

void TabManagerWidget::groupByDomainName(bool useHostName)
{
    QList<BrowserWindow*> windows = mApp->windows();
    int currentWindowIdx = windows.indexOf(getQupZilla());
    if (currentWindowIdx == -1) {
        // getQupZilla() instance is closing
        return;
    }
    windows.move(currentWindowIdx, 0);

    QMap<QString, QTreeWidgetItem*> tabsGroupedByDomain;

    for (int win = 0; win < windows.count(); ++win) {
        BrowserWindow* mainWin = windows.at(win);

        QList<WebTab*> tabs = mainWin->tabWidget()->allTabs();

        for (int tab = 0; tab < tabs.count(); ++tab) {
            WebTab* webTab = tabs.at(tab);
            if (webTab->webView() && m_webPage == webTab->webView()->page()) {
                m_webPage = 0;
                continue;
            }
            QString domain = domainFromUrl(webTab->url(), useHostName);

            if (!tabsGroupedByDomain.contains(domain)) {
                QTreeWidgetItem* groupItem = createEmptyItem(0, false);
                groupItem->setText(0, domain);
                groupItem->setToolTip(0, domain);
                QFont font = groupItem->font(0);
                font.setBold(true);
                groupItem->setFont(0, font);
                tabsGroupedByDomain.insert(domain, groupItem);
            }
            QTreeWidgetItem* groupItem = tabsGroupedByDomain.value(domain);

            QTreeWidgetItem* tabItem = createEmptyItem(groupItem);
            if (webTab == mainWin->weView()->webTab()) {
                QFont font = tabItem->font(0);
                font.setBold(true);
                tabItem->setFont(0, font);
            }
            if (!webTab->isLoading()) {
                if (!webTab->isPinned()) {
                    tabItem->setIcon(0, webTab->icon());
                }
                else {
                    tabItem->setIcon(0, QIcon(":tabmanager/data/tab-pinned.png"));
                }
            }
            else {
                tabItem->setIcon(0, QIcon(":tabmanager/data/tab-loading.png"));
            }
            tabItem->setText(0, webTab->title());
            tabItem->setToolTip(0, webTab->title());

            tabItem->setData(0, UrlRole, webTab->url().toString());
            tabItem->setData(0, WebTabPointerRole, QVariant::fromValue(qobject_cast<QWidget*>(webTab)));
            tabItem->setData(0, QupZillaPointerRole, QVariant::fromValue(qobject_cast<QWidget*>(mainWin)));

            makeWebViewConnections(webTab->webView());
        }
    }

    ui->treeWidget->insertTopLevelItems(0, tabsGroupedByDomain.values());
}

void TabManagerWidget::groupByWindow()
{
    QList<BrowserWindow*> windows = mApp->windows();
    int currentWindowIdx = windows.indexOf(getQupZilla());
    if (currentWindowIdx == -1) {
        return;
    }
    m_isRefreshing = true;

    if (!m_isDefaultWidget) {
        windows.move(currentWindowIdx, 0);
        currentWindowIdx = 0;
    }

    for (int win = 0; win < windows.count(); ++win) {
        BrowserWindow* mainWin = windows.at(win);
        QTreeWidgetItem* winItem = createEmptyItem();
        winItem->setText(0, tr("Window %1").arg(QString::number(win + 1)));
        winItem->setToolTip(0, tr("Double click to switch"));
        if (win == currentWindowIdx) {
            QFont font = winItem->font(0);
            font.setBold(true);
            winItem->setFont(0, font);
        }

        winItem->setData(0, QupZillaPointerRole, QVariant::fromValue(qobject_cast<QWidget*>(mainWin)));
        QList<WebTab*> tabs = mainWin->tabWidget()->allTabs();

        for (int tab = 0; tab < tabs.count(); ++tab) {
            WebTab* webTab = tabs.at(tab);
            if (webTab->webView() && m_webPage == webTab->webView()->page()) {
                m_webPage = 0;
                continue;
            }
            QTreeWidgetItem* tabItem = createEmptyItem(winItem);
            if (webTab == mainWin->weView()->webTab()) {
                QFont font = tabItem->font(0);
                font.setBold(true);
                tabItem->setFont(0, font);
            }
            if (!webTab->isLoading()) {
                if (!webTab->isPinned()) {
                    tabItem->setIcon(0, webTab->icon());
                }
                else {
                    tabItem->setIcon(0, QIcon(":tabmanager/data/tab-pinned.png"));
                }
            }
            else {
                tabItem->setIcon(0, QIcon(":tabmanager/data/tab-loading.png"));
            }
            tabItem->setText(0, webTab->title());
            tabItem->setToolTip(0, webTab->title());

            tabItem->setData(0, UrlRole, webTab->url().toString());
            tabItem->setData(0, WebTabPointerRole, QVariant::fromValue(qobject_cast<QWidget*>(webTab)));
            tabItem->setData(0, QupZillaPointerRole, QVariant::fromValue(qobject_cast<QWidget*>(mainWin)));

            makeWebViewConnections(webTab->webView());
        }
    }
}

BrowserWindow* TabManagerWidget::getQupZilla()
{
    if (m_isDefaultWidget || !p_QupZilla) {
        return mApp->getWindow();
    }
    else {
        return p_QupZilla.data();
    }
}

void TabManagerWidget::makeWebViewConnections(WebView* view)
{
    if (view) {
        connect(view->page(), SIGNAL(loadFinished(bool)), this, SLOT(delayedRefreshTree()));
        connect(view->page(), SIGNAL(loadStarted()), this, SLOT(delayedRefreshTree()));
        connect(view, SIGNAL(titleChanged(QString)), this, SLOT(delayedRefreshTree()));
        connect(view, &WebView::iconChanged, this, [this]() {
            delayedRefreshTree();
        });
    }
}
