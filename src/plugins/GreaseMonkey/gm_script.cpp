/* ============================================================
* GreaseMonkey plugin for QupZilla
* Copyright (C) 2012-2016  David Rosca <nowrep@gmail.com>
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
#include "gm_downloader.h"

#include "qzregexp.h"
#include "delayedfilewatcher.h"
#include "mainapplication.h"

#include <QFile>
#include <QStringList>
#include <QWebEngineScript>
#include <QCryptographicHash>

GM_Script::GM_Script(GM_Manager* manager, const QString &filePath)
    : QObject(manager)
    , m_manager(manager)
    , m_fileWatcher(new DelayedFileWatcher(this))
    , m_namespace("GreaseMonkeyNS")
    , m_startAt(DocumentEnd)
    , m_noframes(false)
    , m_fileName(filePath)
    , m_enabled(true)
    , m_valid(false)
    , m_updating(false)
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

bool GM_Script::noFrames() const
{
    return m_noframes;
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
    return m_include;
}

QStringList GM_Script::exclude() const
{
    return m_exclude;
}

QString GM_Script::script() const
{
    return m_script;
}

QString GM_Script::fileName() const
{
    return m_fileName;
}

QWebEngineScript GM_Script::webScript() const
{
    QWebEngineScript::InjectionPoint injectionPoint;
    switch (startAt()) {
    case DocumentStart:
        injectionPoint = QWebEngineScript::DocumentCreation;
        break;
    case DocumentEnd:
        injectionPoint = QWebEngineScript::DocumentReady;
        break;
    case DocumentIdle:
        injectionPoint = QWebEngineScript::Deferred;
        break;
    default:
        Q_UNREACHABLE();
    }

    QWebEngineScript script;
    script.setName(fullName());
    script.setWorldId(QWebEngineScript::MainWorld);
    script.setInjectionPoint(injectionPoint);
    script.setRunsOnSubFrames(!m_noframes);
    script.setSourceCode(QSL("%1\n%2").arg(m_manager->bootstrapScript(), m_script));
    return script;
}

bool GM_Script::isUpdating()
{
    return m_updating;
}

void GM_Script::updateScript()
{
    if (!m_downloadUrl.isValid() || m_updating)
        return;

    m_updating = true;
    emit updatingChanged(m_updating);

    GM_Downloader *downloader = new GM_Downloader(m_downloadUrl, m_manager);
    downloader->updateScript(m_fileName);
    connect(downloader, &GM_Downloader::finished, this, [this]() {
        m_updating = false;
        emit updatingChanged(m_updating);
    });
    connect(downloader, &GM_Downloader::error, this, [this]() {
        m_updating = false;
        emit updatingChanged(m_updating);
    });
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

static QString toJavaScriptList(const QStringList &patterns)
{
    QString out;
    foreach (const QString &pattern, patterns) {
        QString p;
        if (pattern.startsWith(QL1C('/')) && pattern.endsWith(QL1C('/')) && pattern.size() > 1) {
            p = pattern.mid(1, pattern.size() - 2);
        } else {
            p = pattern;
            p.replace(QL1S("."), QL1S("\\."));
            p.replace(QL1S("*"), QL1S(".*"));
        }
        p = QSL("'%1'").arg(p);
        if (out.isEmpty()) {
            out.append(p);
        } else {
            out.append(QL1C(',') + p);
        }
    }
    return QSL("[%1]").arg(out);
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
    m_noframes = false;
    m_script.clear();
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

    const QString fileData = QString::fromUtf8(file.readAll());

    QzRegExp rx(QSL("(?:^|[\\r\\n])// ==UserScript==(.*)(?:\\r\\n|[\\r\\n])// ==/UserScript==(?:[\\r\\n]|$)"));
    rx.indexIn(fileData);
    QString metadataBlock = rx.cap(1).trimmed();

    if (metadataBlock.isEmpty()) {
        qWarning() << "GreaseMonkey: File does not contain metadata block" << m_fileName;
        return;
    }

    QStringList requireList;
    QzRegExp rxNL(QSL("(?:\\r\\n|[\\r\\n])"));

    const QStringList lines = metadataBlock.split(rxNL, QString::SkipEmptyParts);
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
            m_updateUrl = QUrl(value);
        }
        else if (key == QLatin1String("@downloadURL")) {
            m_downloadUrl = QUrl(value);
        }
        else if (key == QLatin1String("@include") || key == QLatin1String("@match")) {
            m_include.append(value);
        }
        else if (key == QLatin1String("@exclude") || key == QLatin1String("@exclude_match")) {
            m_exclude.append(value);
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
            else if (value == QLatin1String("document-idle")) {
                m_startAt = DocumentIdle;
            }
        }
    }

    if (m_include.isEmpty()) {
        m_include.append(QSL("*"));
    }

    const QString nspace = QCryptographicHash::hash(fullName().toUtf8(), QCryptographicHash::Md4).toHex();
    const QString gmValues = m_manager->valuesScript().arg(nspace);
    const QString runCheck = QString(QL1S("for (var value of %1) {"
                                          "    var re = new RegExp(value);"
                                          "    if (re.test(window.location.href)) {"
                                          "        return;"
                                          "    }"
                                          "}"
                                          "__qz_includes = false;"
                                          "for (var value of %2) {"
                                          "    var re = new RegExp(value);"
                                          "    if (re.test(window.location.href)) {"
                                          "        __qz_includes = true;"
                                          "        break;"
                                          "    }"
                                          "}"
                                          "if (!__qz_includes) {"
                                          "    return;"
                                          "}"
                                          "delete __qz_includes;")).arg(toJavaScriptList(m_exclude), toJavaScriptList(m_include));

    m_script = QSL("(function(){%1\n%2\n%3\n%4\n})();").arg(runCheck, gmValues, m_manager->requireScripts(requireList), fileData);
    m_valid = true;
}
