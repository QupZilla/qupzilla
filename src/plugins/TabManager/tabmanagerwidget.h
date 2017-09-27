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
#ifndef TABMANAGERWIDGET_H
#define TABMANAGERWIDGET_H

#include <QWidget>
#include <QPointer>
#include <QHash>

namespace Ui
{
class TabManagerWidget;
}
class QUrl;
class QTreeWidgetItem;
class BrowserWindow;
class WebPage;
class WebTab;
class WebView;
class TLDExtractor;

class TabManagerWidget : public QWidget
{
    Q_OBJECT

public:
    enum GroupType {
        GroupByWindow = 0,
        GroupByDomain = 1,
        GroupByHost = 2
    };

    explicit TabManagerWidget(BrowserWindow* mainClass, QWidget* parent = 0, bool defaultWidget = false);
    ~TabManagerWidget();

    void closeSelectedTabs(const QHash<BrowserWindow*, WebTab*> &tabsHash);
    void detachSelectedTabs(const QHash<BrowserWindow*, WebTab*> &tabsHash);
    bool bookmarkSelectedTabs(const QHash<BrowserWindow*, WebTab*> &tabsHash);

    void setGroupType(GroupType type);

    static QString domainFromUrl(const QUrl &url, bool useHostName = false);

public slots:
    void delayedRefreshTree(WebPage* p = 0);
    void changeGroupType();

private:
    enum TabDataRole {
        WebTabPointerRole = Qt::UserRole + 10,
        QupZillaPointerRole = Qt::UserRole + 20,
        UrlRole = Qt::UserRole + 30
    };

    QTreeWidgetItem* createEmptyItem(QTreeWidgetItem* parent = 0, bool addToTree = true);
    QTreeWidgetItem* groupByDomainName(bool useHostName = false);
    QTreeWidgetItem* groupByWindow();
    BrowserWindow* getQupZilla();

    void makeWebViewConnections(WebView *view);

    Ui::TabManagerWidget* ui;
    QPointer<BrowserWindow> p_QupZilla;
    WebPage* m_webPage;

    bool m_isRefreshing;
    bool m_refreshBlocked;
    bool m_waitForRefresh;
    bool m_isDefaultWidget;
    GroupType m_groupType;

    QString m_filterText;

    static TLDExtractor* s_tldExtractor;

private slots:
    void refreshTree();
    void processActions();
    void onItemActivated(QTreeWidgetItem* item, int column);
    bool isTabSelected();
    void customContextMenuRequested(const QPoint &pos);
    void filterChanged(const QString &filter, bool force = false);
    void filterBarClosed();

protected:
    bool eventFilter(QObject* obj, QEvent* event);

signals:
    void showSideBySide();
    void groupTypeChanged(TabManagerWidget::GroupType);
};

#endif // TABMANAGERWIDGET_H
