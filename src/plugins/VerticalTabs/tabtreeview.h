/* ============================================================
* VerticalTabs plugin for QupZilla
* Copyright (C) 2018 David Rosca <nowrep@gmail.com>
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
#pragma once

#include <QTreeView>

class QMenu;

class BrowserWindow;

class TabTreeDelegate;

class TabTreeView : public QTreeView
{
    Q_OBJECT
    Q_PROPERTY(int backgroundIndentation READ backgroundIndentation WRITE setBackgroundIndentation)

public:
    explicit TabTreeView(BrowserWindow *window, QWidget *parent = nullptr);

    int backgroundIndentation() const;
    void setBackgroundIndentation(int indentation);

    // In TabBar order
    bool areTabsInOrder() const;
    void setTabsInOrder(bool enable);

    bool haveTreeModel() const;
    void setHaveTreeModel(bool enable);

    void setModel(QAbstractItemModel *model) override;

    void updateIndex(const QModelIndex &index);
    void adjustStyleOption(QStyleOptionViewItem *option);

private:
    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const override;
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>()) override;
    void rowsInserted(const QModelIndex &parent, int start, int end) override;
    bool viewportEvent(QEvent *event) override;

    enum DelegateButton {
        NoButton,
        ExpandButton,
        AudioButton,
        CloseButton
    };

    void initView();
    DelegateButton buttonAt(const QPoint &pos, const QModelIndex &index) const;
    void addMenuActions(QMenu *menu, const QModelIndex &index) const;
    void reverseTraverse(const QModelIndex &root, std::function<void(const QModelIndex&)> callback) const;

    BrowserWindow *m_window;
    TabTreeDelegate *m_delegate;
    DelegateButton m_pressedButton = NoButton;
    QPersistentModelIndex m_pressedIndex;
    QPersistentModelIndex m_hoveredIndex;
    bool m_tabsInOrder = false;
    bool m_haveTreeModel = false;
    int m_backgroundIndentation = 0;
    QString m_expandedSessionKey;
    bool m_initializing = false;
};
