/* ============================================================
* QupZilla - WebKit based browser
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
#ifndef HISTORYVIEW_H
#define HISTORYVIEW_H

#include <QTreeView>

class History;
class HistoryFilterModel;
class HeaderView;

class HistoryView : public QTreeView
{
    Q_OBJECT
public:
    enum OpenBehavior { OpenInCurrentTab, OpenInNewTab };

    explicit HistoryView(QWidget* parent = 0);

    HeaderView* header() const;
    HistoryFilterModel* filterModel() const;

signals:
    void openLink(const QUrl &, HistoryView::OpenBehavior);

public slots:
    void removeItems();

private slots:
    void itemActivated(const QModelIndex &index);
    void itemPressed(const QModelIndex &index);

    void openLinkInCurrentTab();
    void openLinkInNewTab();
    void copyTitle();
    void copyAddress();

protected:
    void contextMenuEvent(QContextMenuEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void drawRow(QPainter* painter, const QStyleOptionViewItem &options, const QModelIndex &index) const;

private:
    History* m_history;
    HistoryFilterModel* m_filterModel;
    HeaderView* m_header;

    QModelIndex m_clickedIndex;
};

#endif // HISTORYVIEW_H
