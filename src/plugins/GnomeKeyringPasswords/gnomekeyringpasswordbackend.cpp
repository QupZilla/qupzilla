/* ============================================================
* GnomeKeyringPasswords - gnome-keyring support plugin for QupZilla
* Copyright (C) 2013-2014  David Rosca <nowrep@gmail.com>
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
#include "gnomekeyringpasswordbackend.h"
#include "gnomekeyringplugin.h"

#include <QDateTime>

extern "C" {
#include "gnome-keyring.h"
}

static PasswordEntry createEntry(GnomeKeyringFound* item)
{
    PasswordEntry entry;
    entry.id = item->item_id;
    entry.password = QString::fromUtf8(item->secret);

    for (unsigned i = 0; i < item->attributes->len; ++i) {
        GnomeKeyringAttribute attr = g_array_index(item->attributes, GnomeKeyringAttribute, i);

        if (strcmp(attr.name, "host") == 0) {
            entry.host = QString::fromUtf8(attr.value.string);
        }
        else if (strcmp(attr.name, "username") == 0) {
            entry.username = QString::fromUtf8(attr.value.string);
        }
        else if (strcmp(attr.name, "data") == 0) {
            entry.data = attr.value.string;
        }
        else if (strcmp(attr.name, "updated") == 0) {
            entry.updated = attr.value.integer;
        }
    }

    entry.data.replace(QByteArray("___PASSWORD-VALUE___"), PasswordManager::urlEncodePassword(entry.password));

    return entry;
}

static GnomeKeyringAttributeList* createAttributes(const PasswordEntry &entry)
{
    GnomeKeyringAttributeList* attributes = gnome_keyring_attribute_list_new();

    gnome_keyring_attribute_list_append_string(attributes, "application", "QupZilla");

    QByteArray value = entry.username.toUtf8();
    gnome_keyring_attribute_list_append_string(attributes, "username", value.constData());

    value = entry.data;
    value.replace(PasswordManager::urlEncodePassword(entry.password), "___PASSWORD-VALUE___");
    gnome_keyring_attribute_list_append_string(attributes, "data", value.constData());

    value = entry.host.toUtf8();
    gnome_keyring_attribute_list_append_string(attributes, "host", value.constData());

    gnome_keyring_attribute_list_append_uint32(attributes, "updated", entry.updated);

    return attributes;
}

GnomeKeyringPasswordBackend::GnomeKeyringPasswordBackend()
    : PasswordBackend()
    , m_loaded(false)
{
}

QString GnomeKeyringPasswordBackend::name() const
{
    return GnomeKeyringPlugin::tr("Gnome Keyring");
}

QVector<PasswordEntry> GnomeKeyringPasswordBackend::getEntries(const QUrl &url)
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
    std::sort(list.begin(), list.end());

    return list;
}

QVector<PasswordEntry> GnomeKeyringPasswordBackend::getAllEntries()
{
    initialize();

    return m_allEntries;
}

void GnomeKeyringPasswordBackend::addEntry(const PasswordEntry &entry)
{
    initialize();

    PasswordEntry stored = entry;
    stored.updated = QDateTime::currentDateTime().toTime_t();

    guint32 itemId;
    GnomeKeyringAttributeList* attributes = createAttributes(stored);

    QByteArray pass = stored.password.toUtf8();
    QByteArray host = stored.host.toUtf8();

    GnomeKeyringResult result = gnome_keyring_item_create_sync(GNOME_KEYRING_DEFAULT,
                                GNOME_KEYRING_ITEM_GENERIC_SECRET,
                                host.constData(),
                                attributes,
                                pass.constData(),
                                TRUE, // Update if exists
                                &itemId);

    gnome_keyring_attribute_list_free(attributes);

    if (result != GNOME_KEYRING_RESULT_OK) {
        qWarning() << "GnomeKeyringPasswordBackend::addEntry Cannot add entry to keyring!";
    }

    stored.id = itemId;

    m_allEntries.append(stored);
}

bool GnomeKeyringPasswordBackend::updateEntry(const PasswordEntry &entry)
{
    initialize();

    // Update item attributes
    GnomeKeyringAttributeList* attributes = createAttributes(entry);

    GnomeKeyringResult result = gnome_keyring_item_set_attributes_sync(GNOME_KEYRING_DEFAULT,
                                entry.id.toUInt(),
                                attributes);

    gnome_keyring_attribute_list_free(attributes);

    if (result != GNOME_KEYRING_RESULT_OK) {
        qWarning() << "GnomeKeyringPasswordBackend::updateEntry Cannot updated entry attributes in keyring!";
        return false;
    }

    // Update secret
    GnomeKeyringItemInfo* info;
    result = gnome_keyring_item_get_info_full_sync(GNOME_KEYRING_DEFAULT, entry.id.toUInt(),
             GNOME_KEYRING_ITEM_INFO_SECRET, &info);

    if (result != GNOME_KEYRING_RESULT_OK) {
        qWarning() << "GnomeKeyringPasswordBackend::updateEntry Cannot get entry info from keyring!";
        return false;
    }

    QByteArray pass = entry.password.toUtf8();
    gnome_keyring_item_info_set_secret(info, pass.constData());

    result = gnome_keyring_item_set_info_sync(GNOME_KEYRING_DEFAULT, entry.id.toUInt(), info);

    gnome_keyring_item_info_free(info);

    if (result != GNOME_KEYRING_RESULT_OK) {
        qWarning() << "GnomeKeyringPasswordBackend::updateEntry Cannot set entry info in keyring!";
        return false;
    }

    int index = m_allEntries.indexOf(entry);

    if (index > -1) {
        m_allEntries[index] = entry;
    }

    return true;
}

void GnomeKeyringPasswordBackend::updateLastUsed(PasswordEntry &entry)
{
    initialize();

    entry.updated = QDateTime::currentDateTime().toTime_t();

    GnomeKeyringAttributeList* attributes = createAttributes(entry);

    GnomeKeyringResult result = gnome_keyring_item_set_attributes_sync(GNOME_KEYRING_DEFAULT,
                                entry.id.toUInt(),
                                attributes);

    gnome_keyring_attribute_list_free(attributes);

    if (result != GNOME_KEYRING_RESULT_OK) {
        qWarning() << "GnomeKeyringPasswordBackend::updateLastUsed Cannot updated entry in keyring!";
        return;
    }

    int index = m_allEntries.indexOf(entry);

    if (index > -1) {
        m_allEntries[index] = entry;
    }
}

void GnomeKeyringPasswordBackend::removeEntry(const PasswordEntry &entry)
{
    initialize();

    GnomeKeyringResult result = gnome_keyring_item_delete_sync(GNOME_KEYRING_DEFAULT, entry.id.toUInt());

    if (result != GNOME_KEYRING_RESULT_OK) {
        qWarning() << "GnomeKeyringPasswordBackend::removeEntry Cannot remove entry from keyring!";
        return;
    }

    int index = m_allEntries.indexOf(entry);

    if (index > -1) {
        m_allEntries.remove(index);
    }
}

void GnomeKeyringPasswordBackend::removeAll()
{
    initialize();

    foreach (const PasswordEntry &entry, m_allEntries) {
        removeEntry(entry);
    }

    m_allEntries.clear();
}

void GnomeKeyringPasswordBackend::initialize()
{
    if (m_loaded) {
        return;
    }

    GList* found;
    GnomeKeyringResult result = gnome_keyring_find_itemsv_sync(GNOME_KEYRING_ITEM_GENERIC_SECRET, &found,
                                "application", GNOME_KEYRING_ATTRIBUTE_TYPE_STRING, "QupZilla",
                                NULL);

    if (result != GNOME_KEYRING_RESULT_OK && result != GNOME_KEYRING_RESULT_NO_MATCH) {
        qWarning() << "GnomeKeyringPasswordBackend::initialize Cannot read items from keyring!";
        return;
    }

    GList* tmp = found;

    while (tmp) {
        GnomeKeyringFound* item = (GnomeKeyringFound*) tmp->data;
        m_allEntries.append(createEntry(item));
        tmp = tmp->next;
    }

    gnome_keyring_found_list_free(found);

    m_loaded = true;
}
