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
#ifndef SETTINGS_H
#define SETTINGS_H

#include <QVariant>

#include "qzcommon.h"

class QSettings;

class QzSettings;

class QUPZILLA_EXPORT Settings
{
public:
    explicit Settings();
    ~Settings();

    static void createSettings(const QString &fileName);
    static void syncSettings();

    static QSettings* globalSettings();
    static QzSettings* staticSettings();

    bool contains(const QString &key) const;
    void remove(const QString &key);

    void setValue(const QString &key, const QVariant &defaultValue = QVariant());
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant());

    void beginGroup(const QString &prefix);
    void endGroup();

    void sync();

private:
    static QSettings* s_settings;
    static QzSettings* s_qzSettings;

    QString m_openedGroup;

};

#endif // SETTINGS_H
