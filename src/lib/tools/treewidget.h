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
#ifndef BOOKMARKSTREEWIDGET_H
#define BOOKMARKSTREEWIDGET_H

#include "qz_namespace.h"

#include <QTreeWidget>

class QT_QUPZILLA_EXPORT TreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit TreeWidget(QWidget* parent = 0);
    enum ItemShowMode { ItemsCollapsed = 0, ItemsExpanded = 1 };
    ItemShowMode defaultItemShowMode() { return m_showMode; }
    void setDefaultItemShowMode(ItemShowMode mode) { m_showMode = mode; }
    QList<QTreeWidgetItem*> allItems();

    bool appendToParentItem(const QString &parentText, QTreeWidgetItem* item);
    bool appendToParentItem(QTreeWidgetItem* parent, QTreeWidgetItem* item);
    bool prependToParentItem(const QString &parentText, QTreeWidgetItem* item);
    bool prependToParentItem(QTreeWidgetItem* parent, QTreeWidgetItem* item);

    void addTopLevelItem(QTreeWidgetItem* item);
    void addTopLevelItems(const QList<QTreeWidgetItem*> &items);
    void insertTopLevelItem(int index, QTreeWidgetItem* item);
    void insertTopLevelItems(int index, const QList<QTreeWidgetItem*> &items);

    void deleteItem(QTreeWidgetItem* item);
    void deleteItems(const QList<QTreeWidgetItem*> &items);

    void setDragDropReceiver(bool enable, QObject* receiver = 0);
    void setMimeType(const QString &mimeType);

signals:
    void itemControlClicked(QTreeWidgetItem* item);
    void itemMiddleButtonClicked(QTreeWidgetItem* item);

    void linkWasDroped(const QUrl &url, const QString &title, const QVariant &imageVariant, const QString &folder, bool* ok);
    void bookmarkParentChanged(int id, const QString &newParent, const QString &oldParent, bool* ok);
    void folderParentChanged(const QString &name, bool isSubfolder, bool* ok);

public slots:
    void filterString(const QString &string);
    void clear();

private slots:
    void sheduleRefresh();

private:
    void mousePressEvent(QMouseEvent* event);
    void iterateAllItems(QTreeWidgetItem* parent);

    Qt::DropActions supportedDropActions() const;
    QStringList mimeTypes() const;
    QMimeData* mimeData(const QList<QTreeWidgetItem*> items) const;
    bool dropMimeData(QTreeWidgetItem* parent, int, const QMimeData* data, Qt::DropAction action);
    void dragEnterEvent(QDragEnterEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);

    bool m_refreshAllItemsNeeded;
    QList<QTreeWidgetItem*> m_allTreeItems;
    ItemShowMode m_showMode;
    QString m_mimeType;
};

#endif // BOOKMARKSTREEWIDGET_H
