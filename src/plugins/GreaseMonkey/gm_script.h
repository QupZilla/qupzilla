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
#ifndef GM_SCRIPT_H
#define GM_SCRIPT_H

#include <QObject>
#include <QIcon>
#include <QUrl>

class QWebEngineScript;

class GM_Manager;

class DelayedFileWatcher;

class GM_Script : public QObject
{
    Q_OBJECT
public:
    explicit GM_Script(GM_Manager* manager, const QString &filePath);

    enum StartAt { DocumentStart, DocumentEnd, DocumentIdle };

    bool isValid() const;
    QString name() const;
    QString nameSpace() const;
    QString fullName() const;

    QString description() const;
    QString version() const;

    QIcon icon() const;
    QUrl iconUrl() const;

    QUrl downloadUrl() const;
    QUrl updateUrl() const;

    StartAt startAt() const;
    bool noFrames() const;

    bool isEnabled() const;
    void setEnabled(bool enable);

    QStringList include() const;
    QStringList exclude() const;
    QStringList require() const;

    QString metaData() const;
    QString fileName() const;

    QWebEngineScript webScript() const;

    bool isUpdating();
    void updateScript();

signals:
    void scriptChanged();
    void updatingChanged(bool updating);

private slots:
    void watchedFileChanged(const QString &file);

private:
    void parseScript();
    void reloadScript();
    void downloadIcon();
    void downloadRequires();

    GM_Manager* m_manager;
    DelayedFileWatcher* m_fileWatcher;

    QString m_name;
    QString m_namespace;
    QString m_description;
    QString m_version;

    QStringList m_include;
    QStringList m_exclude;
    QStringList m_require;

    QIcon m_icon;
    QUrl m_iconUrl;
    QUrl m_downloadUrl;
    QUrl m_updateUrl;
    StartAt m_startAt;
    bool m_noframes;

    QString m_script;
    QString m_fileName;
    bool m_enabled;
    bool m_valid;
    bool m_updating;
};

#endif // GM_SCRIPT_H
