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
#ifndef WEBINSPECTORDOCKWIDGET_H
#define WEBINSPECTORDOCKWIDGET_H

#include <QDockWidget>
#include <QWebInspector>
#include <QPair>
#include <QWeakPointer>

class WebPage;
class QupZilla;
class WebInspectorDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit WebInspectorDockWidget(QupZilla* mainClass);
    ~WebInspectorDockWidget();

    void setPage(WebPage* page) { m_page = page; }

signals:

public slots:
    void tabChanged();

    void close();
    void show();

private:
    QupZilla* p_QupZilla;
    QWeakPointer<QWebInspector> m_inspector;
    WebPage* m_page;
};

#endif // WEBINSPECTORDOCKWIDGET_H
