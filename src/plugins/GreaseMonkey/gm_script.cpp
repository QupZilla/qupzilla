/* ============================================================
* GreaseMonkey plugin for QupZilla
* Copyright (C) 2012-2018 David Rosca <nowrep@gmail.com>
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

#include "delayedfilewatcher.h"
#include "mainapplication.h"
#include "webpage.h"
#include "networkmanager.h"

#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QWebEngineScript>
#include <QCryptographicHash>
#include <QNetworkReply>
#include <QNetworkRequest>

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

QIcon GM_Script::icon() const
{
    return m_icon;
}

QUrl GM_Script::iconUrl() const
{
    return m_iconUrl;
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

QStringList GM_Script::require() const
{
    return m_require;
}

QString GM_Script::fileName() const
{
    return m_fileName;
}

QWebEngineScript GM_Script::webScript() const
{
    QWebEngineScript script;
    script.setSourceCode(QSL("%1\n%2").arg(m_manager->bootstrapScript(), m_script));
    script.setName(fullName());
    script.setWorldId(WebPage::SafeJsWorld);
    script.setRunsOnSubFrames(!m_noframes);
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
    downloadRequires();
}

void GM_Script::watchedFileChanged(const QString &file)
{
    if (m_fileName == file) {
        reloadScript();
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
    m_require.clear();
    m_icon = QIcon();
    m_iconUrl.clear();
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

    const QByteArray fileData = file.readAll();

    bool inMetadata = false;

    QTextStream stream(fileData);
    QString line;
    while (stream.readLineInto(&line)) {
        if (line.startsWith(QL1S("// ==UserScript=="))) {
            inMetadata = true;
        }
        if (line.startsWith(QL1S("// ==/UserScript=="))) {
            break;
        }
        if (!inMetadata) {
            continue;
        }

        if (!line.startsWith(QLatin1String("// @"))) {
            continue;
        }

        line = line.mid(3).replace(QLatin1Char('\t'), QLatin1Char(' '));
        int index = line.indexOf(QLatin1Char(' '));

        const QString key = line.left(index).trimmed();
        const QString value = index > 0 ? line.mid(index + 1).trimmed() : QString();

        if (key.isEmpty()) {
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
            m_require.append(value);
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
        else if (key == QL1S("@icon")) {
            m_iconUrl = QUrl(value);
        }
        else if (key == QL1S("@noframes")) {
            m_noframes = true;
        }
    }

    if (!inMetadata) {
        qWarning() << "GreaseMonkey: File does not contain metadata block" << m_fileName;
        return;
    }

    m_iconUrl = m_downloadUrl.resolved(m_iconUrl);

    if (m_include.isEmpty()) {
        m_include.append(QSL("*"));
    }

    const QString nspace = QCryptographicHash::hash(fullName().toUtf8(), QCryptographicHash::Md4).toHex();
    const QString gmValues = m_manager->valuesScript().arg(nspace);
    m_script = QSL("(function(){%1\n%2\n%3\n})();").arg(gmValues, m_manager->requireScripts(m_require), fileData);
    m_valid = true;

    downloadIcon();
    downloadRequires();
}

void GM_Script::reloadScript()
{
    parseScript();

    m_manager->removeScript(this, false);
    m_manager->addScript(this);

    emit scriptChanged();
}

void GM_Script::downloadIcon()
{
    if (m_iconUrl.isValid()) {
        QNetworkReply *reply = mApp->networkManager()->get(QNetworkRequest(m_iconUrl));
        connect(reply, &QNetworkReply::finished, this, [=]() {
            reply->deleteLater();
            if (reply->error() == QNetworkReply::NoError) {
                m_icon = QPixmap::fromImage(QImage::fromData(reply->readAll()));
            }
        });
    }
}

void GM_Script::downloadRequires()
{
    for (const QString &url : qAsConst(m_require)) {
        if (m_manager->requireScripts({url}).isEmpty()) {
            GM_Downloader *downloader = new GM_Downloader(QUrl(url), m_manager, GM_Downloader::DownloadRequireScript);
            connect(downloader, &GM_Downloader::finished, this, &GM_Script::reloadScript);
        }
    }
}
