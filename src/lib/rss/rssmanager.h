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
#ifndef RSSMANAGER_H
#define RSSMANAGER_H

#include "qz_namespace.h"

#include <QWidget>
#include <QTreeWidget>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QFormLayout>
#include <QWeakPointer>
#include <QDialogButtonBox>
#include <QToolButton>
#include <QInputDialog>

namespace Ui
{
class RSSManager;
}

class QupZilla;
class FollowRedirectReply;
class NetworkManager;
class QT_QUPZILLA_EXPORT RSSManager : public QWidget
{
    Q_OBJECT

public:
    explicit RSSManager(QupZilla* mainClass, QWidget* parent = 0);
    ~RSSManager();

    bool addRssFeed(const QString &address, const QString &title, const QIcon &icon);
    void setMainWindow(QupZilla* window);

public slots:
    void refreshTable();

private slots:
    void optimizeDb();
    void beginToLoadSlot(const QUrl &url);
    void finished();
    void loadFeed(QTreeWidgetItem* item);
    void controlLoadFeed(QTreeWidgetItem* item);
    void addFeed();
    void reloadFeeds();
    void deleteFeed();
    void editFeed();
    void customContextMenuRequested(const QPoint &position);
    void loadFeedInNewTab();

private:
    QupZilla* getQupZilla();
    QList<QPair<FollowRedirectReply*, QUrl> > m_replies;
    NetworkManager* m_networkManager;

    Ui::RSSManager* ui;
    QToolButton* m_reloadButton;
    QWeakPointer<QupZilla> p_QupZilla;
};

#endif // RSSMANAGER_H
