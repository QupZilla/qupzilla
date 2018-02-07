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
#include <QTreeWidgetItem>

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

class TabTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    TabTreeWidget(QWidget* parent = 0);

    Qt::DropActions supportedDropActions() const;
    QStringList mimeTypes() const;
    QMimeData* mimeData(const QList<QTreeWidgetItem*> items) const;
    bool dropMimeData(QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action);

    void setEnableDragTabs(bool enable);

signals:
    void requestRefreshTree();

};

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
    void unloadSelectedTabs(const QHash<BrowserWindow*, WebTab*> &tabsHash);

    void setGroupType(GroupType type);

    static QString domainFromUrl(const QUrl &url, bool useHostName = false);

public slots:
    void delayedRefreshTree(WebPage* p = 0);
    void changeGroupType();

private:
    QTreeWidgetItem* groupByDomainName(bool useHostName = false);
    QTreeWidgetItem* groupByWindow();
    BrowserWindow* getQupZilla();

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

class TabItem : public QObject, public QTreeWidgetItem
{
    Q_OBJECT

public:
    enum StateRole {
        ActiveOrCaptionRole = Qt::UserRole + 1,
        SavedRole = Qt::UserRole + 2
    };

    TabItem(QTreeWidget* treeWidget, bool supportDrag = true, bool isTab = true, QTreeWidgetItem* parent = 0, bool addToTree = true);

    BrowserWindow* window() const;
    void setBrowserWindow(BrowserWindow* window);

    WebTab* webTab() const;
    void setWebTab(WebTab* webTab);

    bool isTab() const;

public slots:
    void updateIcon();
    void setTitle(const QString& title);
    void setIsActiveOrCaption(bool yes);
    void setIsSavedTab(bool yes);

private:
    QTreeWidget* m_treeWidget;
    BrowserWindow* m_window;
    WebTab* m_webTab;
    bool m_isTab;
};

#endif // TABMANAGERWIDGET_H
