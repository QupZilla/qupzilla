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
#ifndef WEBTAB_H
#define WEBTAB_H

#include <QWidget>
#include <QWeakPointer>

#include "qz_namespace.h"

class QVBoxLayout;

class QupZilla;
class LocationBar;
class TabbedWebView;

class QT_QUPZILLA_EXPORT WebTab : public QWidget
{
    Q_OBJECT
public:
    explicit WebTab(QupZilla* mainClass, LocationBar* locationBar);
    ~WebTab();
    TabbedWebView* view();
    bool isPinned();
    void pinTab(int index);
    void setPinned(bool state);

    void setLocationBar(LocationBar* bar);
    LocationBar* locationBar();

    bool inspectorVisible();
    void setInspectorVisible(bool v);

    void disconnectObjects();

private slots:
    void showNotification(QWidget* notif);

private:
    int tabIndex();

    QupZilla* p_QupZilla;
    TabbedWebView* m_view;
    QVBoxLayout* m_layout;
    QWeakPointer<LocationBar> m_locationBar;

    bool m_pinned;
    bool m_inspectorVisible;
};

#endif // WEBTAB_H
