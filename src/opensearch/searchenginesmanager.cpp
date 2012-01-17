/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
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
#include "searchenginesmanager.h"
#include "iconprovider.h"
#include "mainapplication.h"
#include "networkmanager.h"
#include "opensearchreader.h"
#include "opensearchengine.h"
#include "databasewriter.h"
#include "settings.h"

#define ENSURE_LOADED if (!m_settingsLoaded) loadSettings();

QIcon SearchEnginesManager::iconForSearchEngine(const QUrl &url)
{
    QIcon ic = IconProvider::iconFromImage(mApp->iconProvider()->iconForDomain(url));
    if (ic.isNull()) {
        ic = QIcon(":icons/menu/search-icon.png");
    }

    return ic;
}

SearchEnginesManager::SearchEnginesManager()
    : QObject()
    , m_settingsLoaded(false)
    , m_saveScheduled(false)
{
    Settings settings;
    settings.beginGroup("SearchEngines");
    m_startingEngineName = settings.value("activeEngine", "Google").toString();
    settings.endGroup();

    connect(this, SIGNAL(enginesChanged()), this, SLOT(scheduleSave()));
}

void SearchEnginesManager::loadSettings()
{
    m_settingsLoaded = true;

    QSqlQuery query;
    query.exec("SELECT name, icon, url, shortcut, suggestionsUrl FROM search_engines");
    while (query.next()) {
        Engine en;
        en.name = query.value(0).toString();
        en.icon = IconProvider::iconFromBase64(query.value(1).toByteArray());
        en.url = query.value(2).toString();
        en.shortcut = query.value(3).toString();
        en.suggestionsUrl = query.value(4).toString();

        m_allEngines.append(en);
    }

    if (m_allEngines.isEmpty()) {
        restoreDefaults();
    }
}

SearchEngine SearchEnginesManager::engineForShortcut(const QString &shortcut)
{
    Engine returnEngine;
    if (shortcut.isEmpty()) {
        return returnEngine;
    }

    foreach(Engine en, m_allEngines) {
        if (en.shortcut == shortcut) {
            returnEngine = en;
            break;
        }
    }

    return returnEngine;
}

QUrl SearchEnginesManager::searchUrl(const Engine &engine, const QString &string)
{
    ENSURE_LOADED;

    QString url = engine.url;
    url.replace("%s", string);
    return QUrl(url);
}

QUrl SearchEnginesManager::searchUrl(const QString &string)
{
    ENSURE_LOADED;

    QString url = m_activeEngine.url;
    url.replace("%s", string);
    return QUrl(url);
}

void SearchEnginesManager::restoreDefaults()
{
    Engine google;
    google.name = "Google";
    google.icon = QIcon(":icons/menu/google.png");
    google.url = "http://www.google.com/search?client=qupzilla&q=%s";
    google.shortcut = "g";
    google.suggestionsUrl = "http://suggestqueries.google.com/complete/search?output=firefox&q=%s";

    Engine wiki;
    wiki.name = "Wikipedia (en)";
    wiki.icon = QIcon(":/icons/menu/wikipedia.png");
    wiki.url = "http://en.wikipedia.org/wiki/Special:Search?search=%s&fulltext=Search";
    wiki.shortcut = "w";
    wiki.suggestionsUrl = "http://en.wikipedia.org/w/api.php?action=opensearch&search=%s&namespace=0";

    Engine yt;
    yt.name = "YouTube";
    yt.icon = QIcon(":/icons/menu/youtube.png");
    yt.url = "http://www.youtube.com/results?search_query=%s&search=Search";
    yt.shortcut = "yt";
    yt.suggestionsUrl = "http://suggestqueries.google.com/complete/search?ds=yt&output=firefox&q=%s";

    Engine yahoo;
    yahoo.name = "Yahoo!";
    yahoo.icon = QIcon(":/icons/menu/yahoo.png");
    yahoo.url = "http://search.yahoo.com/search?ei=utf-8&fr=sfp&iscqry=&p=%s";
    yahoo.shortcut = "yh";

    addEngine(google, false);
    addEngine(wiki, false);
    addEngine(yt, false);
    addEngine(yahoo, false);

    emit enginesChanged();
}

void SearchEnginesManager::engineChangedImage()
{
    OpenSearchEngine* engine = qobject_cast<OpenSearchEngine*>(sender());

    if (!engine) {
        return;
    }

    foreach(Engine e, m_allEngines) {
        if (e.name == engine->name() && e.url.contains(engine->searchUrl("%s").toString())
                && !engine->image().isNull()) {
            e.icon = QIcon(QPixmap::fromImage(engine->image()));

            m_allEngines.removeOne(e);
            m_allEngines.append(e);

            emit enginesChanged();

            delete engine;
            break;
        }
    }
}

void SearchEnginesManager::editEngine(const Engine &before, const Engine &after)
{
    removeEngine(before);
    addEngine(after);
}

void SearchEnginesManager::addEngine(const Engine &engine, bool emitSignal)
{
    ENSURE_LOADED;

    if (m_allEngines.contains(engine)) {
        return;
    }

    m_allEngines.append(engine);

    if (emitSignal) {
        emit enginesChanged();
    }
}

void SearchEnginesManager::addEngine(OpenSearchEngine* engine)
{
    ENSURE_LOADED;

    Engine en;
    en.name = engine->name();
    en.url = engine->searchUrl("searchstring").toString().replace("searchstring", "%s");
    if (engine->image().isNull()) {
        en.icon = iconForSearchEngine(engine->searchUrl(""));
    }
    else {
        en.icon = QIcon(QPixmap::fromImage(engine->image()));
    }
    en.suggestionsUrl = engine->getSuggestionsUrl();
    en.suggestionsParameters = engine->getSuggestionsParameters();

    addEngine(en);

    connect(engine, SIGNAL(imageChanged()), this, SLOT(engineChangedImage()));
}

void SearchEnginesManager::addEngine(const QUrl &url)
{
    ENSURE_LOADED;

    if (!url.isValid()) {
        return;
    }

    qApp->setOverrideCursor(Qt::WaitCursor);

    QNetworkReply* reply = mApp->networkManager()->get(QNetworkRequest(url));
    reply->setParent(this);
    connect(reply, SIGNAL(finished()), this, SLOT(replyFinished()));
}

void SearchEnginesManager::replyFinished()
{
    qApp->restoreOverrideCursor();

    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        reply->close();
        reply->deleteLater();
        return;
    }

    OpenSearchReader reader;
    OpenSearchEngine* engine = reader.read(reply);
    engine->setNetworkAccessManager(mApp->networkManager());

    reply->close();
    reply->deleteLater();

    if (checkEngine(engine)) {
        addEngine(engine);
        QMessageBox::information(0, tr("Search Engine Added"), tr("Search Engine \"%1\" has been successfully added.").arg(engine->name()));
    }
}

bool SearchEnginesManager::checkEngine(OpenSearchEngine* engine)
{
    if (!engine->isValid()) {
        QString errorString = tr("Search Engine is not valid!");
        QMessageBox::warning(0, tr("Error"), tr("Error while adding Search Engine <br><b>Error Message: </b> %1").arg(errorString));

        return false;
    }

    return true;
}

void SearchEnginesManager::setActiveEngine(const Engine &engine)
{
    ENSURE_LOADED;

    if (!m_allEngines.contains(engine)) {
        return;
    }

    m_activeEngine = engine;
    emit activeEngineChanged();
}

void SearchEnginesManager::removeEngine(const Engine &engine)
{
    ENSURE_LOADED;

    if (!m_allEngines.contains(engine)) {
        return;
    }

    QSqlQuery query;
    query.prepare("DELETE FROM search_engines WHERE name=? AND url=?");
    query.bindValue(0, engine.name);
    query.bindValue(1, engine.url);
    query.exec();

    m_allEngines.removeOne(engine);
    emit enginesChanged();
}

void SearchEnginesManager::setAllEngines(const QList<Engine> &engines)
{
    ENSURE_LOADED;

    m_allEngines = engines;
    emit enginesChanged();
}

QList<SearchEngine> SearchEnginesManager::allEngines()
{
    ENSURE_LOADED;

    return m_allEngines;
}

void SearchEnginesManager::saveSettings()
{
    Settings settings;
    settings.beginGroup("SearchEngines");
    settings.setValue("activeEngine", m_activeEngine.name);
    settings.endGroup();

    if (!m_saveScheduled) {
        return;
    }

    // Well, this is not the best implementation to do as this is taking some time.
    // Actually, it is delaying the quit of app for about a 1 sec on my machine with only
    // 5 engines. Another problem is that deleting rows without VACUUM isn't actually freeing
    // space in database.
    //
    // But as long as user is not playing with search engines every run it is acceptable.

    QSqlQuery query;
    query.exec("DELETE FROM search_engines");

    foreach(Engine en, m_allEngines) {
        query.prepare("INSERT INTO search_engines (name, icon, url, shortcut, suggestionsUrl, suggestionsParameters) VALUES (?, ?, ?, ?, ?, ?)");
        query.bindValue(0, en.name);
        query.bindValue(1, IconProvider::iconToBase64(en.icon));
        query.bindValue(2, en.url);
        query.bindValue(3, en.shortcut);
        query.bindValue(4, en.suggestionsUrl);
        query.bindValue(5, en.suggestionsParameters);

        query.exec();
    }
}
