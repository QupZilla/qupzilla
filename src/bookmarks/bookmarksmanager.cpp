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
#include "mainapplication.h"
#include "bookmarksmanager.h"
#include "ui_bookmarksmanager.h"
#include "qupzilla.h"
#include "locationbar.h"
#include "webview.h"
#include "bookmarkstoolbar.h"
#include "tabwidget.h"
#include "bookmarksmodel.h"
#include "qtwin.h"

//Won't be bad idea to rewrite bookmarks access via bookmarksmodel

BookmarksManager::BookmarksManager(QupZilla* mainClass, QWidget* parent) :
    QWidget(parent)
    ,m_isRefreshing(false)
    ,ui(new Ui::BookmarksManager)
    ,p_QupZilla(mainClass)
    ,m_bookmarksModel(mApp->bookmarks())
{
    ui->setupUi(this);
    //CENTER on scren
    const QRect screen = QApplication::desktop()->screenGeometry();
    const QRect &size = QWidget::geometry();
    QWidget::move( (screen.width()-size.width())/2, (screen.height()-size.height())/2 );

#ifdef Q_WS_WIN
    if (QtWin::isCompositionEnabled()) {
        QtWin::extendFrameIntoClientArea(this);
        ui->gridLayout->setContentsMargins(0, 0, 0, 0);
    }
#endif

    connect(ui->deleteB, SIGNAL(clicked()), this, SLOT(deleteItem()));
    connect(ui->close, SIGNAL(clicked(QAbstractButton*)), this, SLOT(hide()));
    connect(ui->bookmarksTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(itemChanged(QTreeWidgetItem*)));
    connect(ui->addFolder, SIGNAL(clicked()), this, SLOT(addFolder()));
    connect(ui->bookmarksTree, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenuRequested(const QPoint &)));
    connect(ui->bookmarksTree, SIGNAL(itemControlClicked(QTreeWidgetItem*)), this, SLOT(itemControlClicked(QTreeWidgetItem*)));
    //QTimer::singleShot(0, this, SLOT(refreshTable()));
}

QupZilla* BookmarksManager::getQupZilla()
{
    if (!p_QupZilla)
        p_QupZilla = mApp->getWindow();
    return p_QupZilla;
}

void BookmarksManager::setMainWindow(QupZilla* window)
{
    if (window)
        p_QupZilla = window;
}

void BookmarksManager::addFolder()
{
    QString text = QInputDialog::getText(this, tr("Add new folder"), tr("Choose name for new bookmark folder: "));
    if (text.isEmpty())
        return;
    QSqlQuery query;
    query.exec("INSERT INTO folders (name) VALUES ('"+text+"')");
    refreshTable();
}

void BookmarksManager::itemChanged(QTreeWidgetItem* item)
{
    if (!item || m_isRefreshing)
        return;

    QString name = item->text(0);
    QUrl url = QUrl(item->text(1));
    int id = item->whatsThis(1).toInt();

    m_bookmarksModel->editBookmark(id, url, name);
}

void BookmarksManager::itemControlClicked(QTreeWidgetItem* item)
{
    if (!item || item->text(1).isEmpty())
        return;
    getQupZilla()->tabWidget()->addView(QUrl(item->text(1)));
}

void BookmarksManager::loadInNewTab()
{
    if (QAction* action = qobject_cast<QAction*>(sender()))
        getQupZilla()->tabWidget()->addView(action->data().toUrl(), tr("New Tab"), TabWidget::NewNotSelectedTab);
}

void BookmarksManager::deleteItem()
{
    QTreeWidgetItem* item = ui->bookmarksTree->currentItem();
    if (!item)
        return;
    QSqlQuery query;

    if (item->text(1).isEmpty()) { // Delete folder
        QString folder = item->text(0);
        if (folder == tr("Bookmarks In Menu") || folder == tr("Bookmarks In ToolBar"))
            return;

        query.exec("DELETE FROM folders WHERE name='"+folder+"'");
        query.exec("DELETE FROM bookmarks WHERE folder='"+folder+"'");
        delete item;
        return;
    }

    QString id = item->whatsThis(1);

    query.exec("DELETE FROM bookmarks WHERE id="+id);
    delete item;
    getQupZilla()->bookmarksToolbar()->refreshBookmarks();
}

void BookmarksManager::addBookmark(WebView* view)
{
    insertBookmark(view->url(), view->title());
}

void BookmarksManager::moveBookmark()
{
    QTreeWidgetItem* item = ui->bookmarksTree->currentItem();
    if (!item)
        return;
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        m_bookmarksModel->editBookmark(item->whatsThis(1).toInt(), item->text(0), action->data().toString());
    }
    refreshTable();
}

void BookmarksManager::contextMenuRequested(const QPoint &position)
{
    if (!ui->bookmarksTree->itemAt(position))
        return;
    QString link = ui->bookmarksTree->itemAt(position)->text(1);
    if (link.isEmpty())
        return;

    QMenu menu;
    menu.addAction(tr("Open link in actual tab"), getQupZilla(), SLOT(loadActionUrl()))->setData(link);
    menu.addAction(tr("Open link in new tab"), this, SLOT(loadInNewTab()))->setData(link);
    menu.addSeparator();

    QMenu moveMenu;
    moveMenu.setTitle(tr("Move bookmark to folder"));
    moveMenu.addAction(QIcon(":icons/other/unsortedbookmarks.png"), tr("Unsorted Bookmarks"), this, SLOT(moveBookmark()))->setData("unsorted");
    moveMenu.addAction(style()->standardIcon(QStyle::SP_DirOpenIcon), tr("Bookmarks In Menu"), this, SLOT(moveBookmark()))->setData("bookmarksMenu");
    moveMenu.addAction(style()->standardIcon(QStyle::SP_DirOpenIcon), tr("Bookmarks In ToolBar"), this, SLOT(moveBookmark()))->setData("bookmarksToolbar");
    QSqlQuery query;
    query.exec("SELECT name FROM folders");
    while(query.next())
        moveMenu.addAction(style()->standardIcon(QStyle::SP_DirIcon), query.value(0).toString(), this, SLOT(moveBookmark()))->setData(query.value(0).toString());
    menu.addMenu(&moveMenu);

    menu.addSeparator();
    menu.addAction(tr("Close"), this, SLOT(close()));

    //Prevent choosing first option with double rightclick
    QPoint pos = QCursor::pos();
    QPoint p(pos.x(), pos.y()+1);
    menu.exec(p);
}

void BookmarksManager::refreshTable()
{
    m_isRefreshing = true;
    ui->bookmarksTree->setUpdatesEnabled(false);
    ui->bookmarksTree->clear();

    QSqlQuery query;
    QTreeWidgetItem* newItem = new QTreeWidgetItem(ui->bookmarksTree);
    newItem->setText(0, tr("Bookmarks In Menu"));
    newItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
    ui->bookmarksTree->addTopLevelItem(newItem);

    newItem = new QTreeWidgetItem(ui->bookmarksTree);
    newItem->setText(0, tr("Bookmarks In ToolBar"));
    newItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
    ui->bookmarksTree->addTopLevelItem(newItem);

    query.exec("SELECT name FROM folders");
    while(query.next()) {
        newItem = new QTreeWidgetItem(ui->bookmarksTree);
        newItem->setText(0, query.value(0).toString());
        newItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
        ui->bookmarksTree->addTopLevelItem(newItem);
    }

    query.exec("SELECT title, url, id, folder FROM bookmarks");
    while(query.next()) {
        QString title = query.value(0).toString();
        QUrl url = query.value(1).toUrl();
        int id = query.value(2).toInt();
        QString folder = query.value(3).toString();
        QTreeWidgetItem* item;
        if (folder == "bookmarksMenu")
            folder = tr("Bookmarks In Menu");
        if (folder == "bookmarksToolbar")
            folder = tr("Bookmarks In ToolBar");

        if (folder != "unsorted") {
            QList<QTreeWidgetItem*> findParent = ui->bookmarksTree->findItems(folder, 0);
            if (findParent.count() == 1) {
                item = new QTreeWidgetItem(findParent.at(0));
            }else{
                QTreeWidgetItem* newParent = new QTreeWidgetItem(ui->bookmarksTree);
                newParent->setText(0, folder);
                newParent->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
                ui->bookmarksTree->addTopLevelItem(newParent);
                item = new QTreeWidgetItem(newParent);
            }
        } else
            item = new QTreeWidgetItem(ui->bookmarksTree);

        item->setText(0, title);
        item->setText(1, url.toEncoded());
        item->setToolTip(0, title);
        item->setToolTip(1, url.toEncoded());

        item->setWhatsThis(1, QString::number(id));
        item->setIcon(0, LocationBar::icon(url));
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui->bookmarksTree->addTopLevelItem(item);
    }
    ui->bookmarksTree->expandAll();

    ui->bookmarksTree->setUpdatesEnabled(true);
    m_isRefreshing = false;
}

void BookmarksManager::insertBookmark(const QUrl &url, const QString &title)
{
    if (url.isEmpty() || title.isEmpty())
        return;
    QDialog* dialog = new QDialog(getQupZilla());
    QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom, dialog);
    QLabel* label = new QLabel(dialog);
    QLineEdit* edit = new QLineEdit(dialog);
    QComboBox* combo = new QComboBox(dialog);
    QDialogButtonBox* box = new QDialogButtonBox(dialog);
    box->addButton(QDialogButtonBox::Ok);
    box->addButton(QDialogButtonBox::Cancel);
    connect(box, SIGNAL(rejected()), dialog, SLOT(reject()));
    connect(box, SIGNAL(accepted()), dialog, SLOT(accept()));
    layout->addWidget(label);
    layout->addWidget(edit);
    layout->addWidget(combo);
    if (m_bookmarksModel->isBookmarked(url))
        layout->addWidget(new QLabel(tr("<b>Warning: </b>You already have this page bookmarked!")));
    layout->addWidget(box);

    combo->addItem(QIcon(":icons/other/unsortedbookmarks.png"), tr("Unsorted Bookmarks"));
    combo->addItem(style()->standardIcon(QStyle::SP_DirOpenIcon), tr("Bookmarks In Menu"));
    combo->addItem(style()->standardIcon(QStyle::SP_DirOpenIcon), tr("Bookmarks In ToolBar"));
    QSqlQuery query;
    query.exec("SELECT name FROM folders");
    while(query.next())
        combo->addItem(style()->standardIcon(QStyle::SP_DirIcon), query.value(0).toString());

    label->setText(tr("Choose name and location of bookmark."));
    edit->setText(title);
    edit->setCursorPosition(0);
    dialog->setWindowIcon(LocationBar::icon(url));
    dialog->setWindowTitle(tr("Add New Bookmark"));

    QSize size = dialog->size();
    size.setWidth(350);
    dialog->resize(size);
    dialog->exec();
    if (dialog->result() == QDialog::Rejected)
        return;
    if (edit->text().isEmpty())
        return;

    query.prepare("INSERT INTO bookmarks (title, url, folder) VALUES (?,?,?)");
    query.bindValue(0, edit->text());
    query.bindValue(1, url.toString());
    if (combo->currentText() == tr("Bookmarks In Menu"))
        query.bindValue(2,"bookmarksMenu");
    else if (combo->currentText() == tr("Bookmarks In ToolBar"))
        query.bindValue(2,"bookmarksToolbar");
    else if (combo->currentText() == tr("Unsorted Bookmarks"))
        query.bindValue(2, "unsorted");
    else query.bindValue(2, combo->currentText());
    query.exec();

    getQupZilla()->bookmarksToolbar()->refreshBookmarks();
    getQupZilla()->locationBar()->checkBookmark();
    delete dialog;
}

void BookmarksManager::insertAllTabs()
{
    QDialog* dialog = new QDialog(getQupZilla());
    QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom, dialog);
    QLabel* label = new QLabel(dialog);
    QComboBox* combo = new QComboBox(dialog);
    QDialogButtonBox* box = new QDialogButtonBox(dialog);
    box->addButton(QDialogButtonBox::Ok);
    box->addButton(QDialogButtonBox::Cancel);
    connect(box, SIGNAL(rejected()), dialog, SLOT(reject()));
    connect(box, SIGNAL(accepted()), dialog, SLOT(accept()));
    layout->addWidget(label);
    layout->addWidget(combo);
    layout->addWidget(box);

    combo->addItem(QIcon(":icons/other/unsortedbookmarks.png"), tr("Unsorted Bookmarks"));
    combo->addItem(style()->standardIcon(QStyle::SP_DirOpenIcon), tr("Bookmarks In Menu"));
    combo->addItem(style()->standardIcon(QStyle::SP_DirOpenIcon), tr("Bookmarks In ToolBar"));
    QSqlQuery query;
    query.exec("SELECT name FROM folders");
    while(query.next())
        combo->addItem(style()->standardIcon(QStyle::SP_DirIcon), query.value(0).toString());

    label->setText(tr("Choose folder for bookmarks:"));
    dialog->setWindowIcon(QIcon(":/icons/qupzilla.png"));
    dialog->setWindowTitle(tr("Bookmark All Tabs"));

    QSize size = dialog->size();
    size.setWidth(350);
    dialog->resize(size);
    dialog->exec();
    if (dialog->result() == QDialog::Rejected)
        return;

    for (int i = 0; i<getQupZilla()->tabWidget()->count(); i++) {
        WebView* view = getQupZilla()->weView(i);
        if (!view || view->url().isEmpty())
            continue;
        query.prepare("INSERT INTO bookmarks (title, url, folder) VALUES (?,?,?)");
        query.bindValue(0, view->title());
        query.bindValue(1, view->url().toString());
        if (combo->currentText() == tr("Bookmarks In Menu"))
            query.bindValue(2,"bookmarksMenu");
        else if (combo->currentText() == tr("Bookmarks In ToolBar"))
            query.bindValue(2,"bookmarksToolbar");
        else if (combo->currentText() == tr("Unsorted Bookmarks"))
            query.bindValue(2, "unsorted");
        else query.bindValue(2, combo->currentText());
        query.exec();
    }
    getQupZilla()->bookmarksToolbar()->refreshBookmarks();
    getQupZilla()->locationBar()->checkBookmark();
    delete dialog;
}

BookmarksManager::~BookmarksManager()
{
    delete ui;
}
