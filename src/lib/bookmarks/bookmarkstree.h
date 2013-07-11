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
#ifndef BOOKMARKSTREE_H
#define BOOKMARKSTREE_H

#include "treewidget.h"
#include "qz_namespace.h"

class QComboBox;
class WebView;
class TreeWidget;

class QT_QUPZILLA_EXPORT BookmarksTree : public TreeWidget
{
    Q_OBJECT
public:
    enum BookmarkView {
        SideBarView,
        ManagerView,
        ComboFolderView, // I should found a better name ;)
        ExportFolderView // reserved for export functionality!
    };

    BookmarksTree(QWidget* parent = 0);

    BookmarksTree::BookmarkView viewType() {return m_viewType;}
    void setViewType(BookmarksTree::BookmarkView viewType);

protected:
    void drawBranches(QPainter* painter, const QRect &rect, const QModelIndex &index) const;

public slots:
    void refreshTree();
    void activeItemChange(int index, QComboBox* combo = 0, const QString &title = QString(), WebView* = 0);

private:
    BookmarkView m_viewType;

signals:
    void requestNewFolder(QWidget*, QString*, bool, QString, WebView*);
};
#endif // BOOKMARKSTREE_H
