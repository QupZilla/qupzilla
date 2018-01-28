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
#ifndef PASSWORDBACKEND_H
#define PASSWORDBACKEND_H

#include "passwordmanager.h"
#include "qzcommon.h"

class QWidget;

class QUPZILLA_EXPORT PasswordBackend
{
public:
    explicit PasswordBackend();
    virtual ~PasswordBackend() { }

    virtual QString name() const = 0;

    virtual QStringList getUsernames(const QUrl &url);
    virtual QVector<PasswordEntry> getEntries(const QUrl &url) = 0;
    virtual QVector<PasswordEntry> getAllEntries() = 0;

    virtual void addEntry(const PasswordEntry &entry) = 0;
    virtual bool updateEntry(const PasswordEntry &entry) = 0;
    virtual void updateLastUsed(PasswordEntry &entry) = 0;

    virtual void removeEntry(const PasswordEntry &entry) = 0;
    virtual void removeAll() = 0;

    virtual void setActive(bool active);
    bool isActive() const;

    virtual bool hasSettings() const;
    virtual void showSettings(QWidget* parent);

private:
    bool m_active;
};

#endif // PASSWORDBACKEND_H
