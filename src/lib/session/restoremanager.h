/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014 Franz Fellner <alpine.art.de@googlemail.com>
*                         David Rosca <nowrep@gmail.com>
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
#ifndef RESTOREMANAGER_H
#define RESTOREMANAGER_H

#include "webtab.h"
#include "qzcommon.h"

class WebPage;
class RecoveryJsObject;

class QUPZILLA_EXPORT RestoreManager
{
public:
    struct WindowData {
        int currentTab;
        QByteArray windowState;
        QVector<WebTab::SavedTab> tabsState;
    };

    explicit RestoreManager(const QString &file);
    virtual ~RestoreManager();

    bool isValid() const;
    QVector<RestoreManager::WindowData> restoreData() const;

    QObject *recoveryObject(WebPage *page);

    static void createFromFile(const QString &file, QVector<RestoreManager::WindowData> &data);
private:
    void createFromFile(const QString &file);

    RecoveryJsObject *m_recoveryObject;
    QVector<RestoreManager::WindowData> m_data;
};

typedef QVector<RestoreManager::WindowData> RestoreData;

// Hint to QVector to use std::realloc on item moving
Q_DECLARE_TYPEINFO(RestoreManager::WindowData, Q_MOVABLE_TYPE);

#endif // RESTOREMANAGER_H
