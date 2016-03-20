/* ============================================================
* GreaseMonkey plugin for QupZilla
* Copyright (C) 2012  David Rosca <nowrep@gmail.com>
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
#ifndef GM_SETTINGSLISTWIDGET_H
#define GM_SETTINGSLISTWIDGET_H

#include <QListWidget>

class GM_SettingsListDelegate;

class GM_SettingsListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit GM_SettingsListWidget(QWidget* parent = 0);

signals:
    void removeItemRequested(QListWidgetItem* item);
    void updateItemRequested(QListWidgetItem* item);

public slots:

private:
    bool containsRemoveIcon(const QPoint &pos) const;
    bool containsUpdateIcon(const QPoint &pos) const;

    void mousePressEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);

    GM_SettingsListDelegate* m_delegate;

};

#endif // GM_SETTINGSLISTWIDGET_H
