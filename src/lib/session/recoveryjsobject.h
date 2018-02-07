/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2015-2018 David Rosca <nowrep@gmail.com>
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

#ifndef RECOVERYJSOBJECT_H
#define RECOVERYJSOBJECT_H

#include <QObject>
#include <QJsonArray>

class WebPage;
class BrowserWindow;
class RestoreManager;

class RecoveryJsObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QJsonArray restoreData READ restoreData CONSTANT)

public:
    explicit RecoveryJsObject(RestoreManager *manager);

    void setPage(WebPage *page);

    QJsonArray restoreData() const;

public slots:
    void startNewSession();
    void restoreSession(const QStringList &excludeWin, const QStringList &excludeTab);

private:
    void closeTab();

    RestoreManager *m_manager;
    WebPage *m_page;
};

#endif // RECOVERYJSOBJECT_H
