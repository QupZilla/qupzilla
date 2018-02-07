/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2013-2018 David Rosca <nowrep@gmail.com>
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
#ifndef HTML5PERMISSIONSNOTIFICATION_H
#define HTML5PERMISSIONSNOTIFICATION_H

#include <QString>
#include <QPointer>

#include "animatedwidget.h"
#include "webpage.h"

namespace Ui
{
class HTML5PermissionsNotification;
}

class HTML5PermissionsNotification : public AnimatedWidget
{
    Q_OBJECT

public:
    explicit HTML5PermissionsNotification(const QUrl &origin, QWebEnginePage* page, const QWebEnginePage::Feature &feature);
    ~HTML5PermissionsNotification();

private slots:
    void grantPermissions();
    void denyPermissions();

private:
    Ui::HTML5PermissionsNotification* ui;

    QUrl m_origin;
    QPointer<QWebEnginePage> m_page;
    QWebEnginePage::Feature m_feature;
};

#endif // HTML5PERMISSIONSNOTIFICATION_H
