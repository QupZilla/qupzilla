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
#include "passwordmanager.h"
#include "passwordbackends/passwordbackend.h"
#include "passwordbackends/databasepasswordbackend.h"
#include "settings.h"

#include <QVector>

PasswordManager::PasswordManager(QObject* parent)
    : QObject(parent)
    , m_backend(0)
    , m_loaded(false)
    , m_databaseBackend(new DatabasePasswordBackend)
{
    m_backends["database"] = m_databaseBackend;
}

void PasswordManager::loadSettings()
{
    Settings settings;
    settings.beginGroup("PasswordManager");
    QString backendId = settings.value("Backend", "database").toString();
    settings.endGroup();

    m_backend = m_backends[m_backends.contains(backendId) ? backendId : "database"];
    m_backend->setActive(true);
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

void PasswordManager::updateEntry(const PasswordEntry &entry)
{
    ensureLoaded();
    m_backend->updateEntry(entry);
}

void PasswordManager::updateLastUsed(const PasswordEntry &entry)
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
    const QString &key = m_backends.key(backend);
    m_backends.remove(key);
}

QString PasswordManager::createHost(const QUrl &url)
{
    QString host = url.host();

    if (host.isEmpty()) {
        host = url.toString();
    }

    return host;
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
}
