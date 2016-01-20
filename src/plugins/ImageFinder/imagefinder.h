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
#ifndef IMAGEFINDER_H
#define IMAGEFINDER_H

#include <QObject>

class WebView;

class ImageFinder : public QObject
{
    Q_OBJECT
public:
    enum SearchEngine {
        Google = 0,
        Yandex,
        TinEye
    };

    explicit ImageFinder(const QString &settingsFile, QObject *parent = Q_NULLPTR);

    SearchEngine searchEngine() const;
    void setSearchEngine(SearchEngine searchEngine);

    QString searchEngineName() const;

    QUrl getSearchQuery(const QUrl &imageUrl);

private:
    QString m_settingsFile;
    SearchEngine m_searchEngine;
};

#endif // IMAGEFINDER_H
