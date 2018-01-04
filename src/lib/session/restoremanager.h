/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014 Franz Fellner <alpine.art.de@googlemail.com>
* Copyright (C) 2010-2018 David Rosca <nowrep@gmail.com>
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

#include "qzcommon.h"
#include "browserwindow.h"

class WebPage;
class RecoveryJsObject;

struct QUPZILLA_EXPORT RestoreData
{
    QVector<BrowserWindow::SavedWindow> windows;
    QByteArray crashedSession;
    QByteArray closedWindows;

    bool isValid() const;
    void clear();

    friend QUPZILLA_EXPORT QDataStream &operator<<(QDataStream &stream, const RestoreData &data);
    friend QUPZILLA_EXPORT QDataStream &operator>>(QDataStream &stream, RestoreData &data);
};

class QUPZILLA_EXPORT RestoreManager
{
public:
    explicit RestoreManager(const QString &file);
    virtual ~RestoreManager();

    bool isValid() const;
    RestoreData restoreData() const;
    void clearRestoreData();

    QObject *recoveryObject(WebPage *page);

    static bool validateFile(const QString &file);
    static void createFromFile(const QString &file, RestoreData &data);

private:
    void createFromFile(const QString &file);

    RecoveryJsObject *m_recoveryObject;
    RestoreData m_data;
};

#endif // RESTOREMANAGER_H
