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

#if QT_VERSION < 0x050000
#include <qjson/parser.h>
#include <qjson/serializer.h>
#else
#include <QJsonDocument>
#endif

QVariant Json::parse(const QByteArray &data, bool *ok)
{
#if QT_VERSION < 0x050000
    QJson::Parser parser;
    return parser.parse(data, ok);
#else
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (ok) {
        *ok = error.error == QJsonParseError::NoError;
    }

    return doc.toVariant();
#endif
}

QByteArray Json::serialize(const QVariant &variant, bool *ok)
{
#if QT_VERSION < 0x050000
    QJson::Serializer serializer;
    serializer.setIndentMode(QJson::IndentFull);
    return serializer.serialize(variant, ok);
#else
    QJsonDocument doc = QJsonDocument::fromVariant(variant);

    if (ok) {
        *ok = !doc.isNull();
    }

    return doc.toJson(QJsonDocument::Indented);
#endif
}
