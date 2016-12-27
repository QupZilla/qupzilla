/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2016 David Rosca <nowrep@gmail.com>
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

#ifndef WEBSCROLLBARMANAGER_H
#define WEBSCROLLBARMANAGER_H

#include <QObject>
#include <QHash>

class QScrollBar;

class WebView;

class WebScrollBarManager : public QObject
{
    Q_OBJECT

public:
    explicit WebScrollBarManager(QObject *parent = 0);

    void loadSettings();

    void addWebView(WebView *view);
    void removeWebView(WebView *view);

    QScrollBar *scrollBar(Qt::Orientation orientation, WebView *view) const;

    static WebScrollBarManager *instance();

private:
    void createUserScript(int thickness);
    void removeUserScript();
    QSize viewportSize(WebView *view, int thickness) const;

    bool m_enabled = true;
    QString m_scrollbarJs;
    QHash<WebView*, struct ScrollBarData*> m_scrollbars;
};

#endif // WEBSCROLLBARMANAGER_H
