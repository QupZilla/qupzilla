/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca
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
#ifndef SEARCHENGINESMANAGER_H
#define SEARCHENGINESMANAGER_H

#include <QObject>
#include <QIcon>
#include <QSqlQuery>
#include <QList>
#include <QVariant>
#include <QNetworkReply>
#include <QMessageBox>
#include <QSettings>

#include "opensearchengine.h"

class SearchEnginesManager : public QObject
{
    Q_OBJECT
public:

    explicit SearchEnginesManager();

    struct Engine {
        QString name;
        QIcon icon;
        QString url;
        QString shortcut;

        QString suggestionsUrl;
        QByteArray suggestionsParameters;


        bool operator==(const Engine &other)
        {
            return (this->name == other.name &&
                    this->url == other.url);
        }
    };

    QUrl searchUrl(const Engine &engine, const QString &string);
    QUrl searchUrl(const QString &string);

    void addEngine(const QUrl &url);
    void addEngine(OpenSearchEngine* engine);
    void addEngine(const Engine &engine, bool emitSignal = true);

    void removeEngine(const Engine &engine);

    void setActiveEngine(const Engine &engine);
    Engine activeEngine() { return m_activeEngine; }
    void editEngine(const Engine &before, const Engine &after);

    Engine engineForShortcut(const QString &shortcut);

    void setAllEngines(const QList<Engine> &engines);
    QList<Engine> allEngines();

    static QIcon iconForSearchEngine(const QUrl &url);

    QString startingEngineName() { return m_startingEngineName; }

    void saveSettings();
    void restoreDefaults();

signals:
    void enginesChanged();

public slots:

private slots:
    void engineChangedImage();
    void replyFinished();

    void scheduleSave() { m_saveScheduled = true; }

private:
    bool checkEngine(OpenSearchEngine* engine);

    void loadSettings();

    bool m_settingsLoaded;
    bool m_saveScheduled;

    QString m_startingEngineName;
    QList<Engine> m_allEngines;
    Engine m_activeEngine;

};

typedef SearchEnginesManager::Engine SearchEngine;

Q_DECLARE_METATYPE(SearchEnginesManager::Engine)

#endif // SEARCHENGINESMANAGER_H
