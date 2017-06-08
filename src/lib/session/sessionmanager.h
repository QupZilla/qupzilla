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
class QFileInfo;

class QUPZILLA_EXPORT SessionManager : public QObject
{
    Q_OBJECT

public:
    struct SessionMetaData {
        QString name;
        QString filePath;
        bool isActive = false;
        bool isDefault = false;
        bool isBackup = false;
    };

    enum SessionFlag {
        SwitchSession = 1,
        CloneSession = 2,
        ReplaceSession = SwitchSession | 4
    };
    Q_DECLARE_FLAGS(SessionFlags, SessionFlag)

    explicit SessionManager(QObject* parent = 0);

    void loadSettings();
    void saveSettings();

    static QString defaultSessionPath();
    QString lastActiveSessionPath() const;
    QString askSessionFromUser();

    void backupSavedSessions();
    void writeCurrentSession(const QString &filePath);

signals:
    void sessionsMetaDataChanged();

public slots:
    void autoSaveLastSession();
    void openSessionManagerDialog();

private slots:
    void aboutToShowSessionsMenu();
    void sessionsDirectoryChanged();
    void openSession(QString sessionFilePath = QString(), SessionFlags flags = nullptr);
    void renameSession(QString sessionFilePath = QString(), SessionFlags flags = nullptr);
    void saveSession();

    void replaceSession(const QString &filePath);
    void switchToSession(const QString &filePath);
    void cloneSession(const QString &filePath);
    void deleteSession(const QString &filePath);
    void newSession();

    QList<SessionMetaData> sessionMetaData(bool withBackups = true);

private:
    bool isActive(const QString &filePath) const;
    bool isActive(const QFileInfo &fileInfo) const;
    void fillSessionsMetaDataListIfNeeded();

    QList<SessionMetaData> m_sessionsMetaDataList;

    QString m_firstBackupSession;
    QString m_secondBackupSession;
    QString m_lastActiveSessionPath;

    friend class SessionManagerDialog;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(SessionManager::SessionFlags)

#endif // SESSIONMANAGER_H
