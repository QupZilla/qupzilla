/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2013  David Rosca <nowrep@gmail.com>
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
#ifndef AUTOFILLMODEL_H
#define AUTOFILLMODEL_H

#include <QObject>

#include "qz_namespace.h"

class QUrl;
class QWebElement;
class QNetworkRequest;

class QupZilla;
class WebPage;
class PasswordManager;
struct PageFormData;
struct PasswordEntry;

class QT_QUPZILLA_EXPORT AutoFill : public QObject
{
public:
    explicit AutoFill(QObject* parent = 0);

    PasswordManager* passwordManager() const;
    void loadSettings();

    bool isStored(const QUrl &url);
    bool isStoringEnabled(const QUrl &url);
    void blockStoringforUrl(const QUrl &url);

    QVector<PasswordEntry> getFormData(const QUrl &url);
    QVector<PasswordEntry> getAllFormData();

    void updateLastUsed(const PasswordEntry &data);

    void addEntry(const QUrl &url, const QString &name, const QString &pass);
    void addEntry(const QUrl &url, const PageFormData &formData);

    void updateEntry(const QUrl &url, const QString &name, const QString &pass);
    void updateEntry(const PasswordEntry &entry);

    void removeEntry(const PasswordEntry &entry);
    void removeAllEntries();

    void post(const QNetworkRequest &request, const QByteArray &outgoingData);
    QVector<PasswordEntry> completePage(WebPage* page);

    static QByteArray exportPasswords();
    static bool importPasswords(const QByteArray &data);

private:
    PasswordManager* m_manager;
    bool m_isStoring;

};

#endif // AUTOFILLMODEL_H
