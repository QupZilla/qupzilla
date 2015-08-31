/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2015 David Rosca <nowrep@gmail.com>
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

#include "scripts.h"
#include "qztools.h"

#include <QUrlQuery>

QString Scripts::setupWebChannel()
{
    QString source =  QL1S("(function() {"
                           "%1"
                           "function registerExternal(e) {"
                           "    window.external = e;"
                           "    if (window.external) {"
                           "        var event = new Event('_qupzilla_external_created');"
                           "        document.dispatchEvent(event);"
                           "    }"
                           "}"
                           "if (self !== top) {"
                           "    if (top.external)"
                           "        registerExternal(top.external);"
                           "    else"
                           "        top.document.addEventListener('_qupzilla_external_created', function() {"
                           "            registerExternal(top.external);"
                           "        });"
                           "    return;"
                           "}"
                           "new QWebChannel(qt.webChannelTransport, function(channel) {"
                           "    registerExternal(channel.objects.qz_object);"
                           "});"
                           "})()");

    return source.arg(QzTools::readAllFileContents(QSL(":/html/qwebchannel.js")));
}

QString Scripts::setCss(const QString &css)
{
    QString source = QL1S("(function() {"
                          "var css = document.createElement('style');"
                          "css.setAttribute('type', 'text/css');"
                          "css.appendChild(document.createTextNode('%1'));"
                          "document.getElementsByTagName('head')[0].appendChild(css);"
                          "})()");

    QString style = css;
    style.replace(QL1S("'"), QL1S("\\'"));
    style.replace(QL1S("\n"), QL1S("\\n"));
    return source.arg(style);
}

QString Scripts::sendPostData(const QUrl &url, const QByteArray &data)
{
    QString source = QL1S("(function() {"
                          "var form = document.createElement('form');"
                          "form.setAttribute('method', 'POST');"
                          "form.setAttribute('action', '%1');"
                          "var val;"
                          "%2"
                          "form.submit();"
                          "})()");

    QString valueSource = QL1S("val = document.createElement('input');"
                               "val.setAttribute('type', 'hidden');"
                               "val.setAttribute('name', '%1');"
                               "val.setAttribute('value', '%2');"
                               "form.appendChild(val);");

    QString values;
    QUrlQuery query(data);

    for (const QPair<QString, QString> &pair : query.queryItems(QUrl::FullyDecoded)) {
        QString value = pair.first;
        QString key = pair.second;
        value.replace(QL1S("'"), QL1S("\\'"));
        key.replace(QL1S("'"), QL1S("\\'"));
        values.append(valueSource.arg(value, key));
    }

    return source.arg(url.toString(), values);
}
