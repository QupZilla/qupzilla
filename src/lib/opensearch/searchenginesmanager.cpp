/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2017 David Rosca <nowrep@gmail.com>
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
#include "searchenginesdialog.h"
#include "editsearchengine.h"
#include "networkmanager.h"
#include "iconprovider.h"
#include "mainapplication.h"
#include "opensearchreader.h"
#include "opensearchengine.h"
#include "settings.h"
#include "qzsettings.h"
#include "webview.h"
#include "sqldatabase.h"

#include <QNetworkReply>
#include <QMessageBox>
#include <QBuffer>

#include <QUrlQuery>

#define ENSURE_LOADED if (!m_settingsLoaded) loadSettings();

static QIcon iconFromBase64(const QByteArray &data)
{
    QIcon image;
    QByteArray bArray = QByteArray::fromBase64(data);
    QBuffer buffer(&bArray);
    buffer.open(QIODevice::ReadOnly);
    QDataStream in(&buffer);
    in >> image;
    buffer.close();

    if (!image.isNull()) {
        return image;
    }

    return IconProvider::emptyWebIcon();
}

static QByteArray iconToBase64(const QIcon &icon)
{
    QByteArray bArray;
    QBuffer buffer(&bArray);
    buffer.open(QIODevice::WriteOnly);
    QDataStream out(&buffer);
    out << icon;
    buffer.close();
    return bArray.toBase64();
}

SearchEnginesManager::SearchEnginesManager(QObject* parent)
    : QObject(parent)
    , m_settingsLoaded(false)
    , m_saveScheduled(false)
{
    Settings settings;
    settings.beginGroup("SearchEngines");
    m_startingEngineName = settings.value("activeEngine", "DuckDuckGo").toString();
    m_defaultEngineName = settings.value("DefaultEngine", "DuckDuckGo").toString();
    settings.endGroup();

    connect(this, SIGNAL(enginesChanged()), this, SLOT(scheduleSave()));
}

void SearchEnginesManager::loadSettings()
{
    m_settingsLoaded = true;

    QSqlQuery query(SqlDatabase::instance()->database());
    query.exec("SELECT name, icon, url, shortcut, suggestionsUrl, suggestionsParameters, postData FROM search_engines");

    while (query.next()) {
        Engine en;
        en.name = query.value(0).toString();
        en.icon = iconFromBase64(query.value(1).toByteArray());
        en.url = query.value(2).toString();
        en.shortcut = query.value(3).toString();
        en.suggestionsUrl = query.value(4).toString();
        en.suggestionsParameters = query.value(5).toByteArray();
        en.postData = query.value(6).toByteArray();

        m_allEngines.append(en);

        if (en.name == m_defaultEngineName) {
            m_defaultEngine = en;
        }
    }

    if (m_allEngines.isEmpty()) {
        restoreDefaults();
    }

    if (m_defaultEngine.name.isEmpty()) {
        m_defaultEngine = m_allEngines[0];
    }
}

SearchEngine SearchEnginesManager::engineForShortcut(const QString &shortcut)
{
    Engine returnEngine;

    if (shortcut.isEmpty()) {
        return returnEngine;
    }

    foreach (const Engine &en, m_allEngines) {
        if (en.shortcut == shortcut) {
            returnEngine = en;
            break;
        }
    }

    return returnEngine;
}

LoadRequest SearchEnginesManager::searchResult(const Engine &engine, const QString &string)
{
    ENSURE_LOADED;

    // GET search engine
    if (engine.postData.isEmpty()) {
        QByteArray url = engine.url.toUtf8();
        url.replace(QLatin1String("%s"), QUrl::toPercentEncoding(string));

        return LoadRequest(QUrl::fromEncoded(url));
    }

    // POST search engine
    QByteArray data = engine.postData;
    data.replace("%s", QUrl::toPercentEncoding(string));

    return LoadRequest(QUrl::fromEncoded(engine.url.toUtf8()), LoadRequest::PostOperation, data);
}

LoadRequest SearchEnginesManager::searchResult(const QString &string)
{
    ENSURE_LOADED;

    const Engine en = qzSettings->searchWithDefaultEngine ? m_defaultEngine : m_activeEngine;
    return searchResult(en, string);
}

void SearchEnginesManager::restoreDefaults()
{
    Engine duck;
    duck.name = "DuckDuckGo";
    duck.icon = QIcon(":/icons/sites/duck.png");
    duck.url = "https://duckduckgo.com/?q=%s&t=qupzilla";
    duck.shortcut = "d";
    duck.suggestionsUrl = "https://ac.duckduckgo.com/ac/?q=%s&type=list";

    Engine sp;
    sp.name = "StartPage";
    sp.icon = QIcon(":/icons/sites/startpage.png");
    sp.url = "https://startpage.com/do/search";
    sp.postData = "query=%s&cat=web&language=english";
    sp.shortcut = "sp";
    sp.suggestionsUrl = "https://startpage.com/cgi-bin/csuggest?output=json&lang=english&query=%s";

    Engine wiki;
    wiki.name = "Wikipedia (en)";
    wiki.icon = QIcon(":/icons/sites/wikipedia.png");
    wiki.url = "https://en.wikipedia.org/wiki/Special:Search?search=%s&fulltext=Search";
    wiki.shortcut = "w";
    wiki.suggestionsUrl = "https://en.wikipedia.org/w/api.php?action=opensearch&search=%s&namespace=0";

    Engine google;
    google.name = "Google";
    google.icon = QIcon(":icons/sites/google.png");
    google.url = "https://www.google.com/search?client=qupzilla&q=%s";
    google.shortcut = "g";
    google.suggestionsUrl = "https://suggestqueries.google.com/complete/search?output=firefox&q=%s";

    addEngine(duck);
    addEngine(sp);
    addEngine(wiki);
    addEngine(google);

    m_defaultEngine = duck;

    emit enginesChanged();
}

// static
QIcon SearchEnginesManager::iconForSearchEngine(const QUrl &url)
{
    QIcon ic = IconProvider::iconForDomain(url);

    if (ic.isNull()) {
        ic = QIcon::fromTheme(QSL("edit-find"), QIcon(QSL(":icons/menu/search-icon.svg")));
    }

    return ic;
}

void SearchEnginesManager::engineChangedImage()
{
    OpenSearchEngine* engine = qobject_cast<OpenSearchEngine*>(sender());

    if (!engine) {
        return;
    }

    foreach (Engine e, m_allEngines) {
        if (e.name == engine->name() &&
            e.url.contains(engine->searchUrl("%s").toString()) &&
            !engine->image().isNull()
           ) {
            int index = m_allEngines.indexOf(e);
            if (index != -1) {
                m_allEngines[index].icon = QIcon(QPixmap::fromImage(engine->image()));

                emit enginesChanged();

                delete engine;
                break;
            }
        }
    }
}

void SearchEnginesManager::editEngine(const Engine &before, const Engine &after)
{
    removeEngine(before);
    addEngine(after);
}

void SearchEnginesManager::addEngine(const Engine &engine)
{
    ENSURE_LOADED;

    if (m_allEngines.contains(engine)) {
        return;
    }

    m_allEngines.append(engine);

    emit enginesChanged();
}

void SearchEnginesManager::addEngineFromForm(const QVariantMap &formData, WebView *view)
{
    if (formData.isEmpty())
        return;

    const QString method = formData.value(QSL("method")).toString();
    bool isPost = method == QL1S("post");

    QUrl actionUrl = formData.value(QSL("action")).toUrl();

    if (actionUrl.isRelative()) {
        actionUrl = view->url().resolved(actionUrl);
    }

    QUrl parameterUrl = actionUrl;

    if (isPost) {
        parameterUrl = QUrl("http://foo.bar");
    }

    const QString &inputName = formData.value(QSL("inputName")).toString();

    QUrlQuery query(parameterUrl);
    query.addQueryItem(inputName, QSL("SEARCH"));

    const QVariantList &inputs = formData.value(QSL("inputs")).toList();
    foreach (const QVariant &pair, inputs) {
        const QVariantList &list = pair.toList();
        if (list.size() != 2)
            continue;

        const QString &name = list.at(0).toString();
        const QString &value = list.at(1).toString();

        if (name == inputName || name.isEmpty() || value.isEmpty())
            continue;

        query.addQueryItem(name, value);
    }

    parameterUrl.setQuery(query);

    if (!isPost) {
        actionUrl = parameterUrl;
    }

    SearchEngine engine;
    engine.name = view->title();
    engine.icon = view->icon();
    engine.url = actionUrl.toEncoded();

    if (isPost) {
        QByteArray data = parameterUrl.toEncoded(QUrl::RemoveScheme);
        engine.postData = data.contains('?') ? data.mid(data.lastIndexOf('?') + 1) : QByteArray();
        engine.postData.replace((inputName + QL1S("=SEARCH")).toUtf8(), (inputName + QL1S("=%s")).toUtf8());
    } else {
        engine.url.replace(inputName + QL1S("=SEARCH"), inputName + QL1S("=%s"));
    }

    EditSearchEngine dialog(SearchEnginesDialog::tr("Add Search Engine"), view);
    dialog.setName(engine.name);
    dialog.setIcon(engine.icon);
    dialog.setUrl(engine.url);
    dialog.setPostData(engine.postData);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    engine.name = dialog.name();
    engine.icon = dialog.icon();
    engine.url = dialog.url();
    engine.shortcut = dialog.shortcut();
    engine.postData = dialog.postData().toUtf8();

    if (engine.name.isEmpty() || engine.url.isEmpty()) {
        return;
    }

    addEngine(engine);
}

void SearchEnginesManager::addEngine(OpenSearchEngine* engine)
{
    ENSURE_LOADED;

    Engine en;
    en.name = engine->name();
    en.url = engine->searchUrl("searchstring").toString().replace(QLatin1String("searchstring"), QLatin1String("%s"));

    if (engine->image().isNull()) {
        en.icon = iconForSearchEngine(engine->searchUrl(QString()));
    }
    else {
        en.icon = QIcon(QPixmap::fromImage(engine->image()));
    }

    en.suggestionsUrl = engine->getSuggestionsUrl();
    en.suggestionsParameters = engine->getSuggestionsParameters();
    en.postData = engine->getPostData("searchstring").replace("searchstring", "%s");

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

void SearchEnginesManager::setDefaultEngine(const SearchEnginesManager::Engine &engine)
{
    ENSURE_LOADED;

    if (!m_allEngines.contains(engine)) {
        return;
    }

    m_defaultEngine = engine;
    emit defaultEngineChanged();
}

void SearchEnginesManager::removeEngine(const Engine &engine)
{
    ENSURE_LOADED;

    int index = m_allEngines.indexOf(engine);

    if (index < 0) {
        return;
    }

    QSqlQuery query(SqlDatabase::instance()->database());
    query.prepare("DELETE FROM search_engines WHERE name=? AND url=?");
    query.bindValue(0, engine.name);
    query.bindValue(1, engine.url);
    query.exec();

    m_allEngines.remove(index);
    emit enginesChanged();
}

void SearchEnginesManager::setAllEngines(const QVector<Engine> &engines)
{
    ENSURE_LOADED;

    m_allEngines = engines;
    emit enginesChanged();
}

QVector<SearchEngine> SearchEnginesManager::allEngines()
{
    ENSURE_LOADED;

    return m_allEngines;
}

void SearchEnginesManager::saveSettings()
{
    Settings settings;
    settings.beginGroup("SearchEngines");
    settings.setValue("activeEngine", m_activeEngine.name);
    settings.setValue("DefaultEngine", m_defaultEngine.name);
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

    QSqlQuery query(SqlDatabase::instance()->database());
    query.exec("DELETE FROM search_engines");

    foreach (const Engine &en, m_allEngines) {
        query.prepare("INSERT INTO search_engines (name, icon, url, shortcut, suggestionsUrl, suggestionsParameters, postData) VALUES (?, ?, ?, ?, ?, ?, ?)");
        query.addBindValue(en.name);
        query.addBindValue(iconToBase64(en.icon));
        query.addBindValue(en.url);
        query.addBindValue(en.shortcut);
        query.addBindValue(en.suggestionsUrl);
        query.addBindValue(en.suggestionsParameters);
        query.addBindValue(en.postData);

        query.exec();
    }
}
