/* ============================================================
* GreaseMonkey plugin for QupZilla
* Copyright (C) 2012-2014  David Rosca <nowrep@gmail.com>
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

#include "qzregexp.h"
#include "delayedfilewatcher.h"

#include <QFile>
#include <QStringList>
#include <QWebFrame>
#include <QCryptographicHash>

GM_Script::GM_Script(GM_Manager* manager, const QString &filePath)
    : QObject(manager)
    , m_manager(manager)
    , m_fileWatcher(new DelayedFileWatcher(this))
    , m_namespace("GreaseMonkeyNS")
    , m_startAt(DocumentEnd)
    , m_fileName(filePath)
    , m_enabled(true)
    , m_valid(false)
{
    parseScript();

    connect(m_fileWatcher, SIGNAL(delayedFileChanged(QString)), this, SLOT(watchedFileChanged(QString)));
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

QUrl GM_Script::updateUrl() const
{
    return m_updateUrl;
}

GM_Script::StartAt GM_Script::startAt() const
{
    return m_startAt;
}

bool GM_Script::isEnabled() const
{
    return m_valid && m_enabled;
}

void GM_Script::setEnabled(bool enable)
{
    m_enabled = enable;
}

QStringList GM_Script::include() const
{
    QStringList list;

    foreach (const GM_UrlMatcher &matcher, m_include) {
        list.append(matcher.pattern());
    }

    return list;
}

QStringList GM_Script::exclude() const
{
    QStringList list;

    foreach (const GM_UrlMatcher &matcher, m_exclude) {
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
    if (!isEnabled()) {
        return false;
    }

    foreach (const GM_UrlMatcher &matcher, m_exclude) {
        if (matcher.match(urlString)) {
            return false;
        }
    }

    foreach (const GM_UrlMatcher &matcher, m_include) {
        if (matcher.match(urlString)) {
            return true;
        }
    }

    return false;
}

void GM_Script::watchedFileChanged(const QString &file)
{
    if (m_fileName == file) {
        parseScript();

        m_manager->removeScript(this, false);
        m_manager->addScript(this);

        emit scriptChanged();
    }
}

void GM_Script::parseScript()
{
    m_name.clear();
    m_namespace = QSL("GreaseMonkeyNS");
    m_description.clear();
    m_version.clear();
    m_include.clear();
    m_exclude.clear();
    m_downloadUrl.clear();
    m_updateUrl.clear();
    m_startAt = DocumentEnd;
    m_enabled = true;
    m_valid = false;

    QFile file(m_fileName);
    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "GreaseMonkey: Cannot open file for reading" << m_fileName;
        return;
    }

    if (!m_fileWatcher->files().contains(m_fileName)) {
        m_fileWatcher->addPath(m_fileName);
    }

    QString fileData = QString::fromUtf8(file.readAll());

    QzRegExp rx(QSL("// ==UserScript==(.*)// ==/UserScript=="));
    rx.indexIn(fileData);
    QString metadataBlock = rx.cap(1).trimmed();

    if (metadataBlock.isEmpty()) {
        qWarning() << "GreaseMonkey: File does not contain metadata block" << m_fileName;
        return;
    }

    QStringList requireList;

    const QStringList lines = metadataBlock.split(QLatin1Char('\n'), QString::SkipEmptyParts);
    foreach (QString line, lines) {
        if (!line.startsWith(QLatin1String("// @"))) {
            continue;
        }

        line = line.mid(3).replace(QLatin1Char('\t'), QLatin1Char(' '));
        int index = line.indexOf(QLatin1Char(' '));

        if (index < 0) {
            continue;
        }

        const QString key = line.left(index).trimmed();
        const QString value = line.mid(index + 1).trimmed();

        if (key.isEmpty() || value.isEmpty()) {
            continue;
        }

        if (key == QLatin1String("@name")) {
            m_name = value;
        }
        else if (key == QLatin1String("@namespace")) {
            m_namespace = value;
        }
        else if (key == QLatin1String("@description")) {
            m_description = value;
        }
        else if (key == QLatin1String("@version")) {
            m_version = value;
        }
        else if (key == QLatin1String("@updateURL")) {
            m_downloadUrl = QUrl(value);
        }
        else if (key == QLatin1String("@include") || key == QLatin1String("@match")) {
            m_include.append(GM_UrlMatcher(value));
        }
        else if (key == QLatin1String("@exclude") || key == QLatin1String("@exclude_match")) {
            m_exclude.append(GM_UrlMatcher(value));
        }
        else if (key == QLatin1String("@require")) {
            requireList.append(value);
        }
        else if (key == QLatin1String("@run-at")) {
            if (value == QLatin1String("document-end")) {
                m_startAt = DocumentEnd;
            }
            else if (value == QLatin1String("document-start")) {
                m_startAt = DocumentStart;
            }
        }
        else if (key == QLatin1String("@downloadURL") && m_downloadUrl.isEmpty()) {
            m_downloadUrl = QUrl(value);
        }
        else if (key == QLatin1String("@updateURL") && m_updateUrl.isEmpty()) {
            m_updateUrl = QUrl(value);
        }
    }

    if (m_include.isEmpty()) {
        m_include.append(GM_UrlMatcher("*"));
    }

    int index = fileData.indexOf(QLatin1String("// ==/UserScript==")) + 18;
    QString script = fileData.mid(index).trimmed();

    QString jscript("(function(){"
                    "function GM_getValue(name,val){return GM_getValueImpl('%1',name,val);}"
                    "function GM_setValue(name,val){return GM_setValueImpl('%1',name,val);}"
                    "function GM_deleteValue(name){return GM_deleteValueImpl('%1',name);}"
                    "function GM_listValues(){return GM_listValuesImpl('%1');}"
                    "\n%2\n})();");
    QString nspace = QCryptographicHash::hash(fullName().toUtf8(), QCryptographicHash::Md4).toHex();

    script.prepend(m_manager->requireScripts(requireList));
    script = jscript.arg(nspace, script);

    m_script = script;
    m_valid = true;
}
