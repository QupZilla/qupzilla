/* ============================================================
* GreaseMonkey plugin for QupZilla
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
#ifndef GM_JSOBJECT_H
#define GM_JSOBJECT_H

#include <QObject>
#include <QStringList>
#include <QSettings>
#include <QVariant>

class GM_JSObject : public QObject
{
    Q_OBJECT
public:
    explicit GM_JSObject(QObject* parent = 0);
    ~GM_JSObject();

    void setSettingsFile(const QString &name);

public slots:
    QVariant getValue(const QString &nspace, const QString &name, const QVariant &dValue);
    void setValue(const QString &nspace, const QString &name, const QVariant &value);
    void deleteValue(const QString &nspace, const QString &name);
    QStringList listValues(const QString &nspace);

private:
    QSettings* m_settings;
};

#endif // GM_JSOBJECT_H
