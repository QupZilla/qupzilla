/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca <nowrep@gmail.com>
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
#ifndef RSSWIDGET_H
#define RSSWIDGET_H

#include <QWidget>
#include <QMenu>
#include <QPushButton>
#include <QDebug>
#include <QWebFrame>

namespace Ui
{
class RSSWidget;
}

class WebView;
class RSSWidget : public QMenu
{
    Q_OBJECT

public:
    explicit RSSWidget(WebView* view, QWidget* parent = 0);
    ~RSSWidget();

    void showAt(QWidget* _parent);

private slots:
    void addRss();

private:
    Ui::RSSWidget* ui;
    WebView* m_view;
};

#endif // RSSWIDGET_H
