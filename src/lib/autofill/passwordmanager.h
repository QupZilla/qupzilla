/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2013  David Rosca <nowrep@gmail.com>
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

#include "qz_namespace.h"

class PasswordBackend;
class DatabasePasswordBackend;

struct PasswordEntry {
    QVariant id;
    QString host;
    QString username;
    QString password;
    QByteArray data;

    bool isValid() const {
        return !password.isEmpty();
    }
};

class QT_QUPZILLA_EXPORT PasswordManager : public QObject
{
    Q_OBJECT
public:
    explicit PasswordManager(QObject* parent = 0);
    ~PasswordManager();

    void loadSettings();

    QVector<PasswordEntry> getEntries(const QUrl &url);
    QVector<PasswordEntry> getAllEntries();

    void addEntry(const PasswordEntry &entry);
    void updateEntry(const PasswordEntry &entry);
    void updateLastUsed(const PasswordEntry &entry);

    void removeEntry(const PasswordEntry &entry);
    void removeAllEntries();

    bool registerBackend(const QString &id, PasswordBackend* backend);
    void unregisterBackend(PasswordBackend* backend);

    static QString createHost(const QUrl &url);

private:
    void ensureLoaded();

    PasswordBackend* m_backend;
    bool m_loaded;

    DatabasePasswordBackend* m_databaseBackend;
    QHash<QString, PasswordBackend*> m_backends;
};

// Hint to QVector to use std::realloc on item moving
Q_DECLARE_TYPEINFO(PasswordEntry, Q_MOVABLE_TYPE);

Q_DECLARE_METATYPE(PasswordEntry)

#endif // PASSWORDMANAGER_H
