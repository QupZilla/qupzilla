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
#ifndef PASSWORDMANAGER_H
#define PASSWORDMANAGER_H

#include <QObject>
#include <QUrl>
#include <QVariant>

#include "qzcommon.h"

class PasswordBackend;
class DatabasePasswordBackend;
class DatabaseEncryptedPasswordBackend;

struct QUPZILLA_EXPORT PasswordEntry {
    QVariant id;
    QString host;
    QString username;
    QString password;
    QByteArray data;
    int updated;

    PasswordEntry() : updated(-1) { }

    bool isValid() const {
        return !password.isEmpty() && !host.isEmpty();
    }

    bool operator==(const PasswordEntry &other) const {
        return id == other.id;
    }

    bool operator<(const PasswordEntry &other) const {
        return updated > other.updated;
    }

    friend QUPZILLA_EXPORT QDataStream &operator<<(QDataStream &stream, const PasswordEntry &entry);
    friend QUPZILLA_EXPORT QDataStream &operator>>(QDataStream &stream, PasswordEntry &entry);
};

class QUPZILLA_EXPORT PasswordManager : public QObject
{
    Q_OBJECT
public:
    explicit PasswordManager(QObject* parent = 0);
    ~PasswordManager();

    void loadSettings();

    QStringList getUsernames(const QUrl &url);
    QVector<PasswordEntry> getEntries(const QUrl &url);
    QVector<PasswordEntry> getAllEntries();

    void addEntry(const PasswordEntry &entry);
    bool updateEntry(const PasswordEntry &entry);
    void updateLastUsed(PasswordEntry &entry);

    void removeEntry(const PasswordEntry &entry);
    void removeAllEntries();

    QHash<QString, PasswordBackend*> availableBackends();
    PasswordBackend* activeBackend();
    void switchBackend(const QString &backendID);

    bool registerBackend(const QString &id, PasswordBackend* backend);
    void unregisterBackend(PasswordBackend* backend);

    static QString createHost(const QUrl &url);
    static QByteArray urlEncodePassword(const QString &password);

private:
    void ensureLoaded();

    bool m_loaded;

    PasswordBackend* m_backend;
    DatabasePasswordBackend* m_databaseBackend;
    DatabaseEncryptedPasswordBackend* m_databaseEncryptedBackend;

    QHash<QString, PasswordBackend*> m_backends;

signals:
    void passwordBackendChanged();
};

// Hint to QVector to use std::realloc on item moving
Q_DECLARE_TYPEINFO(PasswordEntry, Q_MOVABLE_TYPE);

Q_DECLARE_METATYPE(PasswordEntry)

#endif // PASSWORDMANAGER_H
