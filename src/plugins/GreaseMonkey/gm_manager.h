/* ============================================================
* GreaseMonkey plugin for QupZilla
* Copyright (C) 2013  David Rosca <nowrep@gmail.com>
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
#ifndef GM_MANAGER_H
#define GM_MANAGER_H

#include <QObject>
#include <QStringList>
#include <QPointer>

class QUrl;
class QNetworkRequest;

class GM_Script;
class GM_Settings;

class GM_Manager : public QObject
{
    Q_OBJECT
public:
    explicit GM_Manager(const QString &sPath, QObject* parent = 0);

    void showSettings(QWidget* parent);
    void downloadScript(const QNetworkRequest &request);

    QString settinsPath() const;
    QString scriptsDirectory() const;
    QString requireScripts(const QStringList &urlList) const;

    void unloadPlugin();

    QList<GM_Script*> allScripts() const;
    bool containsScript(const QString &fullName) const;

    void enableScript(GM_Script* script);
    void disableScript(GM_Script* script);

    bool addScript(GM_Script* script);
    bool removeScript(GM_Script* script, bool removeFile = true);

    void showNotification(const QString &message, const QString &title = QString());

    static bool canRunOnScheme(const QString &scheme);

signals:
    void scriptsChanged();

public slots:
    void pageLoadStart();

private slots:
    void load();

private:
    QString m_settingsPath;
    QString m_bootstrap;
    QPointer<GM_Settings> m_settings;

    QStringList m_disabledScripts;
    QList<GM_Script*> m_endScripts;
    QList<GM_Script*> m_startScripts;
};

#endif // GM_MANAGER_H
