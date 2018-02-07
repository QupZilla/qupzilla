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
#include "passwordmanager.h"
#include "passwordbackends/passwordbackend.h"
#include "passwordbackends/databasepasswordbackend.h"
#include "passwordbackends/databaseencryptedpasswordbackend.h"
#include "settings.h"

#include <QVector>
#include <QDataStream>

static const int passwordEntryVersion = 2;

QDataStream &operator <<(QDataStream &stream, const PasswordEntry &entry)
{
    stream << passwordEntryVersion;
    stream << entry.host;
    stream << entry.id;
    stream << entry.username;
    stream << entry.password;
    stream << entry.data;
    stream << entry.updated;

    return stream;
}

QDataStream &operator >>(QDataStream &stream, PasswordEntry &entry)
{
    int version;
    stream >> version;

    if (version != passwordEntryVersion) {
        return stream;
    }

    stream >> entry.host;
    stream >> entry.id;
    stream >> entry.username;
    stream >> entry.password;
    stream >> entry.data;
    stream >> entry.updated;

    return stream;
}

PasswordManager::PasswordManager(QObject* parent)
    : QObject(parent)
    , m_loaded(false)
    , m_backend(0)
    , m_databaseBackend(new DatabasePasswordBackend)
    , m_databaseEncryptedBackend(new DatabaseEncryptedPasswordBackend)
{
    m_backends["database"] = m_databaseBackend;
    m_backends["database-encrypted"] = m_databaseEncryptedBackend;
}

void PasswordManager::loadSettings()
{
    Settings settings;
    settings.beginGroup("PasswordManager");
    QString backendId = settings.value("Backend", "database").toString();
    settings.endGroup();

    if (m_backend) {
        m_backend->setActive(false);
    }
    m_backend = m_backends[m_backends.contains(backendId) ? backendId : "database"];
    m_backend->setActive(true);
}

QStringList PasswordManager::getUsernames(const QUrl &url)
{
    ensureLoaded();
    return m_backend->getUsernames(url);
}

QVector<PasswordEntry> PasswordManager::getEntries(const QUrl &url)
{
    ensureLoaded();
    return m_backend->getEntries(url);
}

QVector<PasswordEntry> PasswordManager::getAllEntries()
{
    ensureLoaded();
    return m_backend->getAllEntries();
}

void PasswordManager::addEntry(const PasswordEntry &entry)
{
    ensureLoaded();
    m_backend->addEntry(entry);
}

bool PasswordManager::updateEntry(const PasswordEntry &entry)
{
    ensureLoaded();
    return m_backend->updateEntry(entry);
}

void PasswordManager::updateLastUsed(PasswordEntry &entry)
{
    ensureLoaded();
    m_backend->updateLastUsed(entry);
}

void PasswordManager::removeEntry(const PasswordEntry &entry)
{
    ensureLoaded();
    m_backend->removeEntry(entry);
}

void PasswordManager::removeAllEntries()
{
    ensureLoaded();
    m_backend->removeAll();
}

QHash<QString, PasswordBackend*> PasswordManager::availableBackends()
{
    ensureLoaded();
    return m_backends;
}

PasswordBackend* PasswordManager::activeBackend()
{
    ensureLoaded();
    return m_backend;
}

void PasswordManager::switchBackend(const QString &backendID)
{
    PasswordBackend* backend = m_backends.value(backendID);

    if (!backend) {
        return;
    }

    m_backend->setActive(false);
    m_backend = backend;
    m_backend->setActive(true);

    Settings settings;
    settings.beginGroup("PasswordManager");
    settings.setValue("Backend", backendID);
    settings.endGroup();

    emit passwordBackendChanged();
}

bool PasswordManager::registerBackend(const QString &id, PasswordBackend* backend)
{
    if (m_backends.contains(id)) {
        return false;
    }

    m_backends[id] = backend;
    return true;
}

void PasswordManager::unregisterBackend(PasswordBackend* backend)
{
    const QString key = m_backends.key(backend);
    m_backends.remove(key);

    if (m_backend == backend) {
        m_backend = m_databaseBackend;
    }
}

QString PasswordManager::createHost(const QUrl &url)
{
    QString host = url.host();

    if (host.isEmpty()) {
        host = url.toString();
    }

    if (url.port() != -1) {
        host.append(QLatin1Char(':'));
        host.append(QString::number(url.port()));
    }

    return host;
}

QByteArray PasswordManager::urlEncodePassword(const QString &password)
{
    // Exclude space to properly decode to +
    QByteArray encodedPass = QUrl::toPercentEncoding(password, " ");
    encodedPass.replace(' ', '+'); // space has to be encoded to +
    encodedPass.replace('~', "%7E"); // ~ is unreserved char, needs to be manually encoded
    return encodedPass;
}

void PasswordManager::ensureLoaded()
{
    if (!m_loaded) {
        loadSettings();
        m_loaded = true;
    }
}

PasswordManager::~PasswordManager()
{
    delete m_databaseBackend;
    delete m_databaseEncryptedBackend;
}
