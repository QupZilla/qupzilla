/* ============================================================
* GreaseMonkey plugin for QupZilla
* Copyright (C) 2012-2013  David Rosca <nowrep@gmail.com>
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
#ifndef GM_SCRIPT_H
#define GM_SCRIPT_H

#include "gm_urlmatcher.h"

#include <QObject>
#include <QList>
#include <QUrl>

class QWebFrame;
class QFileSystemWatcher;

class GM_Manager;
class GM_UrlMatcher;

class GM_Script : public QObject
{
    Q_OBJECT
public:
    explicit GM_Script(GM_Manager* manager, const QString &filePath);

    enum StartAt { DocumentStart, DocumentEnd };

    bool isValid() const;
    QString name() const;
    QString nameSpace() const;
    QString fullName() const;

    QString description() const;
    QString version() const;

    QUrl downloadUrl() const;
    StartAt startAt() const;

    bool isEnabled() const;
    void setEnabled(bool enable);

    QStringList include() const;
    QStringList exclude() const;

    QString script() const;
    QString fileName() const;

    bool match(const QString &urlString);

private slots:
    void watchedFileChanged(const QString &file);

private:
    void parseScript();

    GM_Manager* m_manager;
    QFileSystemWatcher* m_fileWatcher;

    QString m_name;
    QString m_namespace;
    QString m_description;
    QString m_version;

    QList<GM_UrlMatcher> m_include;
    QList<GM_UrlMatcher> m_exclude;

    QUrl m_downloadUrl;
    StartAt m_startAt;

    QString m_script;
    QString m_fileName;
    bool m_enabled;
    bool m_valid;
};

#endif // GM_SCRIPT_H
