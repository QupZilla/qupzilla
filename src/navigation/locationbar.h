/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  nowrep
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
#ifndef LOCATIONBAR_H
#define LOCATIONBAR_H

#include <QLabel>
#include <QCompleter>
#include <QStandardItemModel>
#include <QTreeView>
#include <QHeaderView>
#include <QStandardItem>
#include <QUrl>
#include <QSettings>
#include <QAction>
#include <QToolButton>
#include <QMenu>
#include <QTableView>
#include "lineedit.h"

class QupZilla;
class LineEdit;
class LocationCompleter;
class ClickableLabel;
class BookmarksModel;

class LocationBar : public LineEdit
{
    Q_OBJECT;
public:
    explicit LocationBar(QupZilla* mainClass, QWidget* parent = 0);
    ~LocationBar();
    static QIcon icon(const QUrl &url);

    void loadSettings();

public slots:
    void showUrl(const QUrl &url, bool empty = true);
    void checkBookmark();

private slots:
    void siteIconChanged();
    void setPrivacy(bool state);
    void textEdit();
    void showPopup();
    void bookmarkIconClicked();
    void showSiteInfo();
    void rssIconClicked();

private:
    void focusOutEvent(QFocusEvent* e);
    void mouseDoubleClickEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void dropEvent(QDropEvent* event);

    void showGoButton();
    void hideGoButton();

    ClickableLabel* m_bookmarkButton;
    ClickableLabel* m_goButton;
    ClickableLabel* m_rssIcon;
    QToolButton* m_siteIcon;

    bool m_selectAllOnDoubleClick;
    bool m_addComWithCtrl;
    bool m_addCountryWithAlt;
    QupZilla* p_QupZilla;
    LocationCompleter* m_locationCompleter;
    BookmarksModel* m_bookmarksModel;

    bool m_rssIconVisible;
};

#endif // LOCATIONBAR_H
