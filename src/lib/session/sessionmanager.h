/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2017  Razi Alavizadeh <s.r.alavizadeh@gmail.com>
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
#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include "qzcommon.h"

class QAction;
class QMenu;


class QUPZILLA_EXPORT SessionManager : public QObject
{
    Q_OBJECT

public:
    explicit SessionManager(QObject* parent = 0);

    struct SessionMetaData {
        QString name;
        QString filePath;
    };

    void loadSettings();
    void saveSettings();

    static QString defaultSessionPath();
    QString lastActiveSessionPath() const;
    QString askSessionFromUser();

    void backupSavedSessions();
    void writeCurrentSession(const QString &filePath);

public slots:
    void autoSaveLastSession();

private slots:
    void aboutToShowSessionsMenu();
    void aboutToShowSessionSubmenu();
    void sessionsDirectoryChanged();
    void switchToSession();
    void openSession(QString sessionFilePath = QString(), bool switchSession = false);
    void renameSession(QString &sessionFilePath = QString(), bool clone = false);
    void cloneSession();
    void deleteSession();
    void saveSession();
    void newSession();

private:
    void fillSessionsMetaDataListIfNeeded();

    QList<SessionMetaData> m_sessionsMetaDataList;

    QString m_firstBackupSession;
    QString m_secondBackupSession;
    QString m_lastActiveSessionPath;
};

#endif // SESSIONMANAGER_H
