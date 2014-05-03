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
#ifndef HISTORYTREEVIEW_H
#define HISTORYTREEVIEW_H

#include <QTreeView>

class History;
class HistoryFilterModel;
class HeaderView;

class HistoryTreeView : public QTreeView
{
    Q_OBJECT
public:
    enum ViewType {
        HistoryManagerViewType,
        HistorySidebarViewType
    };

    explicit HistoryTreeView(QWidget* parent = 0);

    ViewType viewType() const;
    void setViewType(ViewType type);

    // Returns empty url if more than one (or zero) urls are selected
    QUrl selectedUrl() const;

    HeaderView* header() const;

signals:
    // Open url in current tab
    void urlActivated(const QUrl &url);
    // Open url in new tab
    void urlCtrlActivated(const QUrl &url);
    // Open url in new window
    void urlShiftActivated(const QUrl &url);
    // Context menu signal with point mapped to global
    void contextMenuRequested(const QPoint &point);

public slots:
    void search(const QString &string);
    void removeSelectedItems();

protected:
    void contextMenuEvent(QContextMenuEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent* event);

    void drawRow(QPainter* painter, const QStyleOptionViewItem &options, const QModelIndex &index) const;

private:
    History* m_history;
    HistoryFilterModel* m_filter;
    HeaderView* m_header;
    ViewType m_type;
};

#endif // HISTORYTREEVIEW_H
