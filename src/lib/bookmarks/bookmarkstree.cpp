/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2012  S. Razi Alavizadeh <s.r.alavizadeh@gmail.com>
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

#include "bookmarkstree.h"
#include "iconprovider.h"
#include "bookmarksmodel.h"

#include <QSqlQuery>
#include <QComboBox>

BookmarksTree::BookmarksTree(QWidget* parent)
    : TreeWidget(parent)
    , m_viewType(ManagerView)
{
}

void BookmarksTree::setViewType(BookmarksTree::BookmarkView viewType)
{
    if (viewType != m_viewType) {
        if (m_viewType == ComboFolderView) {
            setItemsExpandable(true);
            setRootIsDecorated(true);
            setIndentation(20); //QTreeView default indentation
        }
        else if (viewType == ComboFolderView) {
            setItemsExpandable(false);
            setRootIsDecorated(false);
            setIndentation(10);
        }
        m_viewType = viewType;
    }
}

void BookmarksTree::drawBranches(QPainter* painter, const QRect &rect, const QModelIndex &index) const
{
    if (m_viewType == ComboFolderView) {
        return;
    }

    TreeWidget::drawBranches(painter, rect, index);
}

void BookmarksTree::refreshTree()
{
    setUpdatesEnabled(false);
    clear();

    QSqlQuery query;
    QTreeWidgetItem* rootItem = invisibleRootItem();
    if (m_viewType == ExportFolderView) {
        rootItem = new QTreeWidgetItem(this);
        rootItem->setText(0, tr("Bookmarks"));
        rootItem->setIcon(0, qIconProvider->fromTheme("bookmarks-organize"));
        addTopLevelItem(rootItem);
    }

    if (m_viewType == ComboFolderView) {
        QTreeWidgetItem* newItem = new QTreeWidgetItem(rootItem);
        newItem->setText(0, _bookmarksUnsorted);
        newItem->setData(0, Qt::UserRole, "unsorted");
        newItem->setIcon(0, QIcon(":/icons/theme/unsortedbookmarks.png"));
        addTopLevelItem(newItem);
    }

    QTreeWidgetItem* newItem = new QTreeWidgetItem(rootItem);
    newItem->setText(0, _bookmarksMenu);
    newItem->setData(0, Qt::UserRole, "bookmarksMenu");
    newItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
    if (m_viewType != ComboFolderView) {
        newItem->setFlags((newItem->flags() & ~Qt::ItemIsDragEnabled) | Qt::ItemIsDropEnabled);
    }
    addTopLevelItem(newItem);

    QTreeWidgetItem* bookmarksToolbar = 0;
    if (m_viewType != SideBarView) {
        bookmarksToolbar = new QTreeWidgetItem(rootItem);
        bookmarksToolbar->setText(0, _bookmarksToolbar);
        bookmarksToolbar->setData(0, Qt::UserRole, "bookmarksToolbar");
        bookmarksToolbar->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
        if (m_viewType != ComboFolderView) {
            bookmarksToolbar->setFlags((bookmarksToolbar->flags() & ~Qt::ItemIsDragEnabled)
                                       | Qt::ItemIsDropEnabled);
        }
        addTopLevelItem(bookmarksToolbar);
    }

    query.exec("SELECT name FROM folders WHERE subfolder!='yes'");
    while (query.next()) {
        newItem = new QTreeWidgetItem(rootItem);
        newItem->setText(0, query.value(0).toString());
        newItem->setData(0, Qt::UserRole, query.value(0).toString());
        newItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
        newItem->setFlags(newItem->flags() | Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled);
        addTopLevelItem(newItem);
    }
    if (m_viewType == ComboFolderView) {
        QTreeWidgetItem* newFolder = new QTreeWidgetItem(rootItem);
        newFolder->setText(0, tr("New Folder..."));
        newFolder->setData(0, Qt::UserRole + 12, "NEW_FOLDER");
        newFolder->setIcon(0, style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    }

    if (m_viewType != ComboFolderView) {
        query.exec("SELECT title, url, id, folder, icon FROM bookmarks");
        while (query.next()) {
            QString title = query.value(0).toString();
            QUrl url = query.value(1).toUrl();
            int id = query.value(2).toInt();
            QString folder = query.value(3).toString();
            QIcon icon = qIconProvider->iconFromImage(QImage::fromData(query.value(4).toByteArray()));
            QTreeWidgetItem* item;
            if (folder == QLatin1String("bookmarksMenu")) {
                folder = _bookmarksMenu;
            }
            if (folder == QLatin1String("bookmarksToolbar")) {
                folder = _bookmarksToolbar;
            }

            if (folder != QLatin1String("unsorted")) {
                QList<QTreeWidgetItem*> findParent = findItems(folder, 0);
                if (findParent.count() != 1) {
                    continue;
                }

                item = new QTreeWidgetItem(findParent.at(0));

            }
            else {
                item = new QTreeWidgetItem(rootItem);
            }

            item->setText(0, title);
            item->setText(1, url.toEncoded());

            if (m_viewType != SideBarView) {
                item->setToolTip(0, title);
                item->setToolTip(1, url.toEncoded());
                // Qt::ItemIsEditable just Manager!!
                item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled);
            }
            else {
                item->setToolTip(0, url.toEncoded());
                item->setFlags(item->flags() | Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled);
            }

            item->setData(0, Qt::UserRole + 10, id);
            item->setData(0, Qt::UserRole + 11, url);
            item->setIcon(0, icon);
            addTopLevelItem(item);
        }
    }

    if (m_viewType != SideBarView) {
        query.exec("SELECT name FROM folders WHERE subfolder='yes'");
        while (query.next()) {
            newItem = new QTreeWidgetItem(bookmarksToolbar);
            newItem->setText(0, query.value(0).toString());
            newItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
            if (m_viewType != ComboFolderView && m_viewType != ExportFolderView) {
                newItem->setFlags(newItem->flags() | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
                QSqlQuery query2;
                query2.prepare("SELECT title, url, id, icon FROM bookmarks WHERE folder=?");
                query2.addBindValue(query.value(0).toString());
                query2.exec();
                while (query2.next()) {
                    QString title = query2.value(0).toString();
                    QUrl url = query2.value(1).toUrl();
                    int id = query2.value(2).toInt();
                    QIcon icon = qIconProvider->iconFromImage(QImage::fromData(query2.value(3).toByteArray()));
                    QTreeWidgetItem* item = new QTreeWidgetItem(newItem);

                    item->setText(0, title);
                    item->setText(1, url.toEncoded());
                    item->setToolTip(0, title);
                    item->setToolTip(1, url.toEncoded());

                    item->setData(0, Qt::UserRole + 10, id);
                    item->setData(0, Qt::UserRole + 11, url);
                    item->setIcon(0, icon);
                    item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled);
                }
            }
        }
    }

    expandAll();
    setUpdatesEnabled(true);
}

void BookmarksTree::activeItemChange(int index, QComboBox* combo, const QString &title, WebView* view)
{
    if (!combo) {
        combo = qobject_cast<QComboBox*>(sender());
    }
    if (!combo) {
        return;
    }
    QString data = combo->itemData(index, Qt::UserRole + 12).toString();
    if (data == "NEW_FOLDER") {
        if (combo->parentWidget()->objectName() == "BookmarksWidget") {
            emit requestNewFolder(this, 0, true, title, view);
        }
        else {
            QString folder;
            emit requestNewFolder(this, &folder, false, QString(), 0);
            if (!folder.isEmpty()) {
                int ind = combo->findText(folder);
                // QComboBox::find() returns index related to the item's parent
                if (ind == -1) {
                    QModelIndex rootIndex = combo->rootModelIndex();
                    combo->setRootModelIndex(combo->model()->index(combo->findText(_bookmarksToolbar), 0));
                    combo->setCurrentIndex(combo->findText(folder));
                    combo->setRootModelIndex(rootIndex);
                }
                else {
                    combo->setCurrentIndex(ind);
                }
            }
            else {
                combo->setCurrentIndex(0);
            }
        }
    }
}
