/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
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
#include "locationcompleter.h"
#include "locationcompleterdelegate.h"
#include "locationbar.h"
#include "iconprovider.h"
#include "mainapplication.h"

#include <QStandardItemModel>
#include <QTreeView>
#include <QHeaderView>
#include <QSqlQuery>

LocationCompleter::LocationCompleter(QObject* parent)
    : QCompleter(parent)
{
    QStandardItemModel* completeModel = new QStandardItemModel();

    setModel(completeModel);
    m_treeView = new CompleterTreeView();

    setPopup(m_treeView);
    m_treeView->setRootIsDecorated(false);
    m_treeView->header()->hide();
    m_treeView->header()->setStretchLastSection(false);
    m_treeView->header()->setResizeMode(0, QHeaderView::Stretch);
    m_treeView->header()->resizeSection(1, 0);

    m_treeView->setItemDelegateForColumn(0, new LocationCompleterDelegate(m_treeView));

    setCompletionMode(QCompleter::PopupCompletion);
    setCaseSensitivity(Qt::CaseInsensitive);
    setWrapAround(true);
    setCompletionColumn(1);
    setMaxVisibleItems(6);
}

QStringList LocationCompleter::splitPath(const QString &path) const
{
    Q_UNUSED(path);
    return QStringList("");
#if 0
    QStringList returned = QCompleter::splitPath(path);
    QStringList returned2;
    QSqlQuery query;
    query.exec("SELECT url FROM history WHERE title LIKE '%" + path + "%' OR url LIKE '%" + path + "%' ORDER BY count DESC LIMIT 1");
    if (query.next()) {
        QString url = query.value(0).toString();
        bool titleSearching = false;
        if (!url.contains(path)) {
            titleSearching = true;
        }
        QString prefix = url.mid(0, url.indexOf(path));
        foreach(const QString & string, returned) {
            if (titleSearching) {
                returned2.append(url);
            }
            else {
                returned2.append(prefix + string);
            }
        }
        return returned2;
    }
    else {
        foreach(const QString & string, returned)
        returned2.append("http://www.google.com/search?client=qupzilla&q=" + string);
        return returned2;
    }
#endif
}

void LocationCompleter::showMostVisited()
{
    QSqlQuery query;
    query.exec("SELECT title, url FROM history ORDER BY count DESC LIMIT 15");
    int i = 0;
    QStandardItemModel* cModel = qobject_cast<QStandardItemModel*>(model());

    cModel->clear();
    while (query.next()) {
        QStandardItem* iconText = new QStandardItem();
        QStandardItem* findUrl = new QStandardItem();
        QString url = query.value(1).toUrl().toEncoded();

        iconText->setIcon(_iconForUrl(query.value(1).toUrl()).pixmap(16, 16));
        iconText->setText(query.value(0).toString());
        iconText->setData(query.value(1), Qt::UserRole);

        findUrl->setText(url);
        QList<QStandardItem*> items;
        items.append(iconText);
        items.append(findUrl);
        cModel->insertRow(i, items);
        i++;
    }

    m_treeView->header()->setResizeMode(0, QHeaderView::Stretch);
    m_treeView->header()->resizeSection(1, 0);

    popup()->setMinimumHeight(190);

    QCompleter::complete();
}

void LocationCompleter::refreshCompleter(const QString &string)
{
    int limit = string.size() < 3 ? 25 : 15;
    int i = 0;
    QString searchString = QString("%%1%").arg(string);

    QStandardItemModel* cModel = qobject_cast<QStandardItemModel*>(model());
    cModel->clear();

    QSqlQuery query;
    query.prepare("SELECT title, url, icon FROM bookmarks WHERE title LIKE ? OR url LIKE ? LIMIT ?");
    query.addBindValue(searchString);
    query.addBindValue(searchString);
    query.addBindValue(limit);
    query.exec();

    while (query.next()) {
        QStandardItem* iconText = new QStandardItem();
        QStandardItem* findUrl = new QStandardItem();
        QString url = query.value(1).toUrl().toEncoded();

        iconText->setIcon(IconProvider::iconFromImage(QImage::fromData(query.value(2).toByteArray())));
        iconText->setText(query.value(0).toString());
        iconText->setData(query.value(1), Qt::UserRole);

        findUrl->setText(url);
        QList<QStandardItem*> items;
        items.append(iconText);
        items.append(findUrl);
        cModel->insertRow(i, items);
        i++;
    }

    query.prepare("SELECT title, url FROM history WHERE title LIKE ? OR url LIKE ? ORDER BY count DESC LIMIT ?");
    query.addBindValue(searchString);
    query.addBindValue(searchString);
    query.addBindValue(limit - i);
    query.exec();

    while (query.next()) {
        QStandardItem* iconText = new QStandardItem();
        QStandardItem* findUrl = new QStandardItem();
        QString url = query.value(1).toUrl().toEncoded();

        iconText->setIcon(_iconForUrl(query.value(1).toUrl()).pixmap(16, 16));
        iconText->setText(query.value(0).toString());
        iconText->setData(query.value(1), Qt::UserRole);

        findUrl->setText(url);
        QList<QStandardItem*> items;
        items.append(iconText);
        items.append(findUrl);
        cModel->insertRow(i, items);
        i++;
    }

    m_treeView->header()->setResizeMode(0, QHeaderView::Stretch);
    m_treeView->header()->resizeSection(1, 0);

    if (i > 6) {
        m_treeView->setMinimumHeight(6 * m_treeView->rowHeight());
    }
    else {
        m_treeView->setMinimumHeight(0);
    }

    m_treeView->setUpdatesEnabled(true);
}
