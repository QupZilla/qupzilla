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
#ifndef RSSMANAGER_H
#define RSSMANAGER_H

#include "qzcommon.h"

#include <QWidget>
#include <QTreeWidget>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QFormLayout>
#include <QPointer>
#include <QDialogButtonBox>
#include <QToolButton>
#include <QInputDialog>

namespace Ui
{
class RSSManager;
}

class BrowserWindow;
class FollowRedirectReply;
class NetworkManager;
class QUPZILLA_EXPORT RSSManager : public QWidget
{
    Q_OBJECT

    friend class BrowsingLibrary;
public:
    explicit RSSManager(BrowserWindow* window, QWidget* parent = 0);
    ~RSSManager();

    bool addRssFeed(QWidget* parent, const QUrl &url, const QString &title, const QIcon &icon);
    void setMainWindow(BrowserWindow* window);

public slots:
    void refreshTable();

private slots:
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
    BrowserWindow* getQupZilla();
    void deleteAllTabs();

    QList<QPair<FollowRedirectReply*, QUrl> > m_replies;
    NetworkManager* m_networkManager;

    Ui::RSSManager* ui;
    QToolButton* m_reloadButton;
    QPointer<BrowserWindow> m_window;
};

#endif // RSSMANAGER_H
