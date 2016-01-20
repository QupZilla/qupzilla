/* ============================================================
* ImageFinder plugin for QupZilla
* Copyright (C) 2016 Vladislav Tronko <innermous@gmail.com>
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
#include "imagefinder.h"

#include <QApplication>
#include <QSettings>
#include <QUrl>

#include "qzcommon.h"

ImageFinder::ImageFinder(const QString &settingsFile, QObject *parent)
    : QObject(parent)
    , m_settingsFile(settingsFile)
    , m_searchEngine(SearchEngine::Google)
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);
    settings.beginGroup(QSL("ImageFinder"));

    m_searchEngine = static_cast<SearchEngine>(settings.value(QSL("SearchEngine")).toInt());

    settings.endGroup();
}

ImageFinder::SearchEngine ImageFinder::searchEngine() const
{
    return m_searchEngine;
}

void ImageFinder::setSearchEngine(ImageFinder::SearchEngine searchEngine)
{
    m_searchEngine = searchEngine;

    QSettings settings(m_settingsFile, QSettings::IniFormat);
    settings.beginGroup(QSL("ImageFinder"));
    settings.setValue(QSL("SearchEngine"), m_searchEngine);
    settings.endGroup();
}

QString ImageFinder::searchEngineName(SearchEngine engine) const
{
    if (engine == SearchEngine::None)
        engine = m_searchEngine;
    QStringList searchEngines;
    searchEngines << QSL("Google") << QSL("Yandex") << QSL("TinEye");

    return searchEngines.at(engine);
}

QUrl ImageFinder::getSearchQuery(const QUrl &imageUrl, SearchEngine engine)
{
    if (engine == SearchEngine::None)
        engine = m_searchEngine;
    switch (engine)
    {
    case SearchEngine::Google:
        return QUrl(QSL("https://www.google.com/searchbyimage?site=search&image_url=%1").arg(imageUrl.toString()));
        break;

    case SearchEngine::Yandex:
        return QUrl(QSL("https://yandex.com/images/search?&img_url=%1&rpt=imageview").arg(imageUrl.toString()));
        break;

    case SearchEngine::TinEye:
        return QUrl(QSL("http://www.tineye.com/search?url=%1").arg(imageUrl.toString()));
        break;

    default: return QUrl();
    }
}
