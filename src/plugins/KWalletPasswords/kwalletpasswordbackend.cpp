/* ============================================================
* KWalletPasswords - KWallet support plugin for QupZilla
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
#include "kwalletpasswordbackend.h"
#include "kwalletplugin.h"
#include "mainapplication.h"
#include "browserwindow.h"

#include <QDateTime>

#include <KWallet>

static PasswordEntry decodeEntry(const QByteArray &data)
{
    QDataStream stream(data);
    PasswordEntry entry;
    stream >> entry;
    return entry;
}

static QByteArray encodeEntry(const PasswordEntry &entry)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << entry;
    return data;
}

KWalletPasswordBackend::KWalletPasswordBackend()
    : PasswordBackend()
    , m_wallet(0)
{
}

QString KWalletPasswordBackend::name() const
{
    return KWalletPlugin::tr("KWallet");
}

QVector<PasswordEntry> KWalletPasswordBackend::getEntries(const QUrl &url)
{
    initialize();

    const QString host = PasswordManager::createHost(url);

    QVector<PasswordEntry> list;

    foreach (const PasswordEntry &entry, m_allEntries) {
        if (entry.host == host) {
            list.append(entry);
        }
    }

    // Sort to prefer last updated entries
    qSort(list.begin(), list.end());

    return list;
}

QVector<PasswordEntry> KWalletPasswordBackend::getAllEntries()
{
    initialize();

    return m_allEntries;
}

void KWalletPasswordBackend::addEntry(const PasswordEntry &entry)
{
    initialize();

    PasswordEntry stored = entry;
    stored.id = QString("%1/%2").arg(entry.host, entry.username);
    stored.updated = QDateTime::currentDateTime().toTime_t();

    m_wallet->writeEntry(stored.id.toString(), encodeEntry(stored));
    m_allEntries.append(stored);
}

bool KWalletPasswordBackend::updateEntry(const PasswordEntry &entry)
{
    initialize();

    m_wallet->removeEntry(entry.id.toString());
    m_wallet->writeEntry(entry.id.toString(), encodeEntry(entry));

    int index = m_allEntries.indexOf(entry);

    if (index > -1) {
        m_allEntries[index] = entry;
    }

    return true;
}

void KWalletPasswordBackend::updateLastUsed(PasswordEntry &entry)
{
    initialize();

    m_wallet->removeEntry(entry.id.toString());

    entry.updated = QDateTime::currentDateTime().toTime_t();

    m_wallet->writeEntry(entry.id.toString(), encodeEntry(entry));

    int index = m_allEntries.indexOf(entry);

    if (index > -1) {
        m_allEntries[index] = entry;
    }
}

void KWalletPasswordBackend::removeEntry(const PasswordEntry &entry)
{
    initialize();

    m_wallet->removeEntry(entry.id.toString());

    int index = m_allEntries.indexOf(entry);

    if (index > -1) {
        m_allEntries.remove(index);
    }
}

void KWalletPasswordBackend::removeAll()
{
    initialize();

    m_allEntries.clear();

    m_wallet->removeFolder("QupZilla");
    m_wallet->createFolder("QupZilla");
}

void KWalletPasswordBackend::initialize()
{
    if (m_wallet) {
        return;
    }

    WId wid = 0;
    BrowserWindow *w = mApp->getWindow();
    if (w && w->window()) {
        wid = w->window()->winId();
    }
    m_wallet = KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(), wid);

    if (!m_wallet) {
        qWarning() << "KWalletPasswordBackend::initialize Cannot open wallet!";
        return;
    }

    if (!m_wallet->hasFolder("QupZilla") && !m_wallet->createFolder("QupZilla")) {
        qWarning() << "KWalletPasswordBackend::initialize Cannot create folder \"QupZilla\"!";
        return;
    }

    if (!m_wallet->setFolder("QupZilla")) {
        qWarning() << "KWalletPasswordBackend::initialize Cannot set folder \"QupZilla\"!";
        return;
    }

    QMap<QString, QByteArray> entries;
    if (m_wallet->readEntryList("*", entries) != 0) {
        qWarning() << "KWalletPasswordBackend::initialize Cannot read entries!";
        return;
    }

    QMap<QString, QByteArray>::const_iterator i = entries.constBegin();
    while (i != entries.constEnd()) {
        PasswordEntry entry = decodeEntry(i.value());
        if (entry.isValid()) {
            m_allEntries.append(entry);
        }
        ++i;
    }
}

KWalletPasswordBackend::~KWalletPasswordBackend()
{
    delete m_wallet;
}
