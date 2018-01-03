/* ============================================================
* GreaseMonkey plugin for QupZilla
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
#include "gm_jsobject.h"

#include <QApplication>
#include <QClipboard>

GM_JSObject::GM_JSObject(QObject* parent)
    : QObject(parent)
    , m_settings(0)
{
}

void GM_JSObject::setSettingsFile(const QString &name)
{
    if (m_settings) {
        m_settings->sync();
        delete m_settings;
    }

    m_settings = new QSettings(name, QSettings::IniFormat);
}

QVariant GM_JSObject::getValue(const QString &nspace, const QString &name, const QVariant &dValue)
{
    QString valueName = QString("GreaseMonkey-%1/%2").arg(nspace, name);
    QString savedValue = m_settings->value(valueName, dValue).toString();

    if (savedValue.isEmpty()) {
        return dValue;
    }

    QString actualValue = savedValue.mid(1).trimmed();
    if (actualValue.isEmpty()) {
        return dValue;
    }

    switch (savedValue.at(0).toLatin1()) {
    case 'b':
        return QVariant(actualValue == QLatin1String("true"));

    case 'i': {
        bool ok;
        int val = actualValue.toInt(&ok);
        return ok ? QVariant(val) : dValue;
    }

    case 's':
        return actualValue;

    default:
        break;
    }

    return dValue;
}

bool GM_JSObject::setValue(const QString &nspace, const QString &name, const QVariant &value)
{
    QString savedValue;

    switch (value.type()) {
    case QVariant::Bool:
        savedValue = value.toBool() ? "btrue" : "bfalse";
        break;

    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    case QVariant::Double:
        savedValue = "i" + QString::number(value.toInt());
        break;

    case QVariant::String:
        savedValue = "s" + value.toString();
        break;

    default:
        return false;
    }

    QString valueName = QString("GreaseMonkey-%1/%2").arg(nspace, name);
    m_settings->setValue(valueName, savedValue);
    return true;
}

bool GM_JSObject::deleteValue(const QString &nspace, const QString &name)
{
    QString valueName = QString("GreaseMonkey-%1/%2").arg(nspace, name);
    m_settings->remove(valueName);
    return true;
}

QStringList GM_JSObject::listValues(const QString &nspace)
{
    QString nspaceName = QString("GreaseMonkey-%1").arg(nspace);

    m_settings->beginGroup(nspaceName);
    QStringList keys = m_settings->allKeys();
    m_settings->endGroup();

    return keys;
}

void GM_JSObject::setClipboard(const QString &text)
{
    QApplication::clipboard()->setText(text);
}

GM_JSObject::~GM_JSObject()
{
    if (m_settings) {
        m_settings->sync();
        delete m_settings;
    }
}
