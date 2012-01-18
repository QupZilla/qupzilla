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
#ifndef LOCATIONBAR_H
#define LOCATIONBAR_H

#include <QLabel>
#include <QCompleter>
#include <QStandardItemModel>
#include <QTreeView>
#include <QHeaderView>
#include <QStandardItem>
#include <QUrl>
#include <QAction>
#include <QToolButton>
#include <QMenu>
#include <QTableView>
#include <QShortcut>

#include "lineedit.h"

class QupZilla;
class LineEdit;
class LocationCompleter;
class ClickableLabel;
class BookmarkIcon;
class WebView;
class LocationBarSettings;
class SiteIcon;
class GoIcon;
class RssIcon;
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
    WebView* webView() { return m_webView; }

signals:
    void loadUrl(const QUrl &url);

public slots:
    void showUrl(const QUrl &url, bool empty = true);
    virtual void setText(const QString &text);

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
    void keyReleaseEvent(QKeyEvent* event);
    void dropEvent(QDropEvent* event);

    QUrl createUrl();

    void showGoButton();
    void hideGoButton();

    BookmarkIcon* m_bookmarkIcon;
    GoIcon* m_goIcon;
    RssIcon* m_rssIcon;
    SiteIcon* m_siteIcon;

    QupZilla* p_QupZilla;
    WebView* m_webView;
    LocationCompleter* m_locationCompleter;
    LocationBarSettings* m_locationBarSettings;

    bool m_rssIconVisible;
    bool m_holdingAlt;
};

#endif // LOCATIONBAR_H
