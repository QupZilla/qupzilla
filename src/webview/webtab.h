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
#ifndef WEBTAB_H
#define WEBTAB_H

#include <QWidget>
#include <QLayout>
#include <QPointer>
#include "webview.h"

class QupZilla;
class WebTab : public QWidget
{
    Q_OBJECT
public:
    explicit WebTab(QupZilla* mainClass, QWidget* parent = 0);
    ~WebTab();
    WebView* view() { return m_view; }

private slots:
    void showNotification(QWidget* notif);

private:
    QupZilla* p_QupZilla;
    QPointer<WebView> m_view;
    QVBoxLayout* m_layout;
};

#endif // WEBTAB_H
