/* ============================================================
* QupZilla - Qt web browser
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
#ifndef AUTOFILL_H
#define AUTOFILL_H

#include <QObject>
#include <QPointer>

#include "qzcommon.h"

class QUrl;
class QWebEnginePage;

class WebPage;
class BrowserWindow;
class PasswordManager;
class AutoFillNotification;
struct PageFormData;
struct PasswordEntry;

struct PageFormData {
    QString username;
    QString password;
    QByteArray postData;

    bool isValid() const {
        return !password.isEmpty();
    }
};

class QUPZILLA_EXPORT AutoFill : public QObject
{
    Q_OBJECT

public:
    explicit AutoFill(QObject* parent = 0);

    PasswordManager* passwordManager() const;
    void loadSettings();

    bool isStored(const QUrl &url);
    bool isStoringEnabled(const QUrl &url);
    void blockStoringforUrl(const QUrl &url);

    QVector<PasswordEntry> getFormData(const QUrl &url);
    QVector<PasswordEntry> getAllFormData();

    void updateLastUsed(PasswordEntry &data);

    void addEntry(const QUrl &url, const QString &name, const QString &pass);
    void addEntry(const QUrl &url, const PageFormData &formData);

    void updateEntry(const QUrl &url, const QString &name, const QString &pass);
    bool updateEntry(const PasswordEntry &entry);

    void removeEntry(const PasswordEntry &entry);
    void removeAllEntries();

    void saveForm(WebPage *page, const QUrl &frameUrl, const PageFormData &formData);
    QStringList completePage(WebPage *page, const QUrl &frameUrl);

    QByteArray exportPasswords();
    bool importPasswords(const QByteArray &data);

private:
    PasswordManager* m_manager;
    bool m_isStoring = false;
    bool m_isAutoComplete = false;
    QPointer<AutoFillNotification> m_lastNotification;
    WebPage *m_lastNotificationPage = nullptr;

};

#endif // AUTOFILL_H
