/* ============================================================
* GreaseMonkey plugin for QupZilla
* Copyright (C) 2012  David Rosca <nowrep@gmail.com>
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
#include "gm_script.h"
#include "gm_manager.h"

#include <QFile>
#include <QRegExp>
#include <QStringList>
#include <QWebFrame>
#include <QDebug>
#include <QElapsedTimer>

GM_Script::GM_Script(GM_Manager* manager, const QString &filePath)
    : m_manager(manager)
    , m_namespace("GreaseMonkeyNS")
    , m_startAt(DocumentEnd)
    , m_fileName(filePath)
    , m_enabled(true)
    , m_valid(false)
{
    parseScript(filePath);
}

bool GM_Script::isValid() const
{
    return m_valid;
}

QString GM_Script::name() const
{
    return m_name;
}

QString GM_Script::nameSpace() const
{
    return m_namespace;
}

QString GM_Script::fullName() const
{
    return QString("%1/%2").arg(m_namespace, m_name);
}

QString GM_Script::description() const
{
    return m_description;
}

QString GM_Script::version() const
{
    return m_version;
}

QUrl GM_Script::downloadUrl() const
{
    return m_downloadUrl;
}

GM_Script::StartAt GM_Script::startAt() const
{
    return m_startAt;
}

bool GM_Script::isEnabled() const
{
    return m_enabled;
}

void GM_Script::setEnabled(bool enable)
{
    m_enabled = enable;
}

QStringList GM_Script::include() const
{
    QStringList list;

    foreach(const GM_UrlMatcher & matcher, m_include) {
        list.append(matcher.pattern());
    }

    return list;
}

QStringList GM_Script::exclude() const
{
    QStringList list;

    foreach(const GM_UrlMatcher & matcher, m_exclude) {
        list.append(matcher.pattern());
    }

    return list;
}

QString GM_Script::script() const
{
    return m_script;
}

QString GM_Script::fileName() const
{
    return m_fileName;
}

bool GM_Script::match(const QString &urlString)
{
    if (!m_enabled) {
        return false;
    }

    foreach(const GM_UrlMatcher & matcher, m_exclude) {
        if (matcher.match(urlString)) {
            return false;
        }
    }

    foreach(const GM_UrlMatcher & matcher, m_include) {
        if (matcher.match(urlString)) {
            return true;
        }
    }

    return false;
}

void GM_Script::parseScript(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "GreaseMonkey: Cannot open file for reading" << filePath;
        return;
    }

    QString fileData = QString::fromUtf8(file.readAll());

    QRegExp rx("// ==UserScript==(.*)// ==/UserScript==");
    rx.indexIn(fileData);
    QString metadataBlock = rx.cap(1).trimmed();

    if (metadataBlock.isEmpty()) {
        qWarning() << "GreaseMonkey: File does not contain metadata block" << filePath;
        return;
    }

    QStringList requireList;

    const QStringList &lines = metadataBlock.split('\n');
    foreach(QString line, lines) {
        if (!line.startsWith("// @")) {
            continue;
        }

        line = line.mid(3).replace('\t', ' ');
        int index = line.indexOf(' ');

        if (index < 0) {
            continue;
        }

        const QString &key = line.left(index).trimmed();
        const QString &value = line.mid(index + 1).trimmed();

        // Ignored values:
        //  @resource
        //  @unwrap

        if (key.isEmpty() || value.isEmpty()) {
            continue;
        }

        if (key == "@name") {
            m_name = value;
        }
        else if (key == "@namespace") {
            m_namespace = value;
        }
        else if (key == "@description") {
            m_description = value;
        }
        else if (key == "@version") {
            m_version = value;
        }
        else if (key == "@updateURL") {
            m_downloadUrl = QUrl(value);
        }
        else if (key == "@include" || key == "@match") {
            m_include.append(GM_UrlMatcher(value));
        }
        else if (key == "@exclude" || key == "@exclude_match") {
            m_exclude.append(GM_UrlMatcher(value));
        }
        else if (key == "@require") {
            requireList.append(value);
        }
        else if (key == "@run-at") {
            if (value == "document-end") {
                m_startAt = DocumentEnd;
            }
            else if (value == "document-start") {
                m_startAt = DocumentStart;
            }
        }
        else if (key == "@downloadURL" && m_downloadUrl.isEmpty()) {
            m_downloadUrl = QUrl(value);
        }
    }

    if (m_include.isEmpty()) {
        m_include.append(GM_UrlMatcher("*"));
    }

    int index = fileData.indexOf("// ==/UserScript==") + 18;
    QString script = fileData.mid(index).trimmed();

    script.prepend(m_manager->requireScripts(requireList));
    script = QString("(function(){%1})();").arg(script);

    m_script = script;
    m_valid = !script.isEmpty();
}
