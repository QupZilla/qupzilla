/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca
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
class BookmarkIcon;
class WebView;
class LocationBarSettings;
class ToolButton;
class LocationBar : public LineEdit
{
    Q_OBJECT
    Q_PROPERTY(QSize fixedsize READ size WRITE setFixedSize)
    Q_PROPERTY(int fixedwidth READ width WRITE setFixedWidth)
    Q_PROPERTY(int fixedheight READ height WRITE setFixedHeight)

public:
    explicit LocationBar(QupZilla* mainClass);
    ~LocationBar();

    void setWebView(WebView* view) { m_webView = view; }

public slots:
    void showUrl(const QUrl &url, bool empty = true);

private slots:
    void siteIconChanged();
    void setPrivacy(bool state);
    void textEdit();
    void showMostVisited();
    void showSiteInfo();
    void rssIconClicked();
    void urlEnter();
    void clearIcon();
    void showRSSIcon(bool state);

    void updatePlaceHolderText();

private:
    void focusOutEvent(QFocusEvent* e);
    void mouseDoubleClickEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void dropEvent(QDropEvent* event);

    void showGoButton();
    void hideGoButton();

    BookmarkIcon* m_bookmarkIcon;
    ClickableLabel* m_goButton;
    ClickableLabel* m_rssIcon;
    ToolButton* m_siteIcon;

    QupZilla* p_QupZilla;
    WebView* m_webView;
    LocationCompleter* m_locationCompleter;
    LocationBarSettings* m_locationBarSettings;

    bool m_rssIconVisible;
};

#endif // LOCATIONBAR_H
