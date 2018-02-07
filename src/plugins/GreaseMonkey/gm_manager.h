/* ============================================================
* GreaseMonkey plugin for QupZilla
* Copyright (C) 2013-2018 David Rosca <nowrep@gmail.com>
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
#include <QHash>

class QUrl;
class QWebFrame;

class BrowserWindow;
class GM_Script;
class GM_JSObject;
class GM_Settings;
class GM_Icon;

class GM_Manager : public QObject
{
    Q_OBJECT
public:
    explicit GM_Manager(const QString &sPath, QObject* parent = 0);
    ~GM_Manager();

    void showSettings(QWidget* parent);
    void downloadScript(const QUrl &url);

    QString settinsPath() const;
    QString scriptsDirectory() const;
    QString requireScripts(const QStringList &urlList) const;
    QString bootstrapScript() const;
    QString valuesScript() const;

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
    void mainWindowCreated(BrowserWindow* window);
    void mainWindowDeleted(BrowserWindow* window);

private slots:
    void load();
    void scriptChanged();

private:
    QString m_settingsPath;
    QString m_bootstrapScript;
    QString m_valuesScript;
    QPointer<GM_Settings> m_settings;

    QStringList m_disabledScripts;
    GM_JSObject *m_jsObject;
    QList<GM_Script*> m_scripts;

    QHash<BrowserWindow*, GM_Icon*> m_windows;
};

#endif // GM_MANAGER_H
