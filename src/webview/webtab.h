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
class LocationBar;
class WebTab : public QWidget
{
    Q_OBJECT
public:
    explicit WebTab(QupZilla* mainClass, LocationBar* locationBar);
    ~WebTab();
    WebView* view() { return m_view; }
    bool isPinned() { return m_pinned; }
    void pinTab(int index);
    void setPinned(bool state) { m_pinned = state; }

    void setLocationBar(LocationBar* bar) { m_locationBar = bar; }
    LocationBar* locationBar() { return m_locationBar; }

    bool inspectorVisible() { return m_inspectorVisible; }
    void setInspectorVisible(bool v) { m_inspectorVisible = v; }

private slots:
    void showNotification(QWidget* notif);

private:
    int tabIndex();

    QupZilla* p_QupZilla;
    QPointer<WebView> m_view;
    QVBoxLayout* m_layout;
    LocationBar* m_locationBar;

    bool m_pinned;
    bool m_inspectorVisible;
};

#endif // WEBTAB_H
