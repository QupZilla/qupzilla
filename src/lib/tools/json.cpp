/* ===========================================================
* QupZilla - WebKit based browser
* Copyright (C) 2014  David Rosca <nowrep@gmail.com>
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
#include "json.h"

#include <QScriptEngine>
#include <QScriptValueIterator>

// Class based on http://stackoverflow.com/a/15805783

Json::Json()
    : m_engine(0)
    , m_ok(true)
{
}

Json::~Json()
{
    delete m_engine;
}

QVariant Json::parse(const QString &data)
{
    delete m_engine;
    m_engine = new QScriptEngine();

    QString jsonData = QString("(%1)").arg(data);
    QScriptValue obj = m_engine->evaluate(jsonData);
    m_ok = !obj.isError() && obj.isObject();

    return decodeInner(obj);
}

QString Json::serialize(const QVariant &variant)
{
    delete m_engine;
    m_engine = new QScriptEngine();

    m_engine->evaluate("function toString() { return JSON.stringify(this, null, ' ') }");

    QScriptValue toString = m_engine->globalObject().property("toString");
    QScriptValue obj = encodeInner(variant.toMap());
    QScriptValue result = toString.call(obj);

    m_ok = !obj.isError() && obj.isObject();
    return result.toString();
}

bool Json::ok() const
{
    return m_ok;
}

QMap<QString, QVariant> Json::decodeInner(QScriptValue object)
{
    QMap<QString, QVariant> map;
    QScriptValueIterator it(object);

    while (it.hasNext()) {
        it.next();

        if (it.value().isArray()) {
            map.insert(it.name(), QVariant(decodeInnerToList(it.value())));
        }
        else if (it.value().isNumber()) {
            map.insert(it.name(), QVariant(it.value().toNumber()));
        }
        else if (it.value().isString()) {
            map.insert(it.name(), QVariant(it.value().toString()));
        }
        else if (it.value().isNull()) {
            map.insert(it.name(), QVariant());
        }
        else if (it.value().isObject()) {
            map.insert(it.name(), QVariant(decodeInner(it.value())));
        }
    }

    return map;
}

QList<QVariant> Json::decodeInnerToList(QScriptValue arrayValue)
{
    QList<QVariant> list;
    QScriptValueIterator it(arrayValue);

    while (it.hasNext()) {
        it.next();

        if (it.name() == QLatin1String("length")) {
            continue;
        }

        if (it.value().isArray()) {
            list.append(QVariant(decodeInnerToList(it.value())));
        }
        else if (it.value().isNumber()) {
            list.append(QVariant(it.value().toNumber()));
        }
        else if (it.value().isString()) {
            list.append(QVariant(it.value().toString()));
        }
        else if (it.value().isNull()) {
            list.append(QVariant());
        }
        else if (it.value().isObject()) {
            list.append(QVariant(decodeInner(it.value())));
        }
    }

    return list;
}

QScriptValue Json::encodeInner(const QMap<QString, QVariant> &map)
{
    QScriptValue obj = m_engine->newObject();
    QMapIterator<QString, QVariant> i(map);

    while (i.hasNext()) {
        i.next();

        if (i.value().type() == QVariant::String) {
            obj.setProperty(i.key(), i.value().toString());
        }
        else if (i.value().type() == QVariant::Int) {
            obj.setProperty(i.key(), i.value().toInt());
        }
        else if (i.value().type() == QVariant::Double) {
            obj.setProperty(i.key(), i.value().toDouble());
        }
        else if (i.value().type() == QVariant::List) {
            obj.setProperty(i.key(), qScriptValueFromSequence(m_engine, i.value().toList()));
        }
        else if (i.value().type() == QVariant::Map) {
            obj.setProperty(i.key(), encodeInner(i.value().toMap()));
        }
    }

    return obj;
}
