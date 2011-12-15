/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca <nowrep@gmail.com>
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
#include "chromeimporter.h"
#include "globalfunctions.h"

ChromeImporter::ChromeImporter(QObject* parent)
    : QObject(parent)
    , m_error(false)
    , m_errorString(tr("No Error"))
{
}

void ChromeImporter::setFile(const QString &path)
{
    m_path = path;
}

bool ChromeImporter::openFile()
{
    m_file.setFileName(m_path);

    if (!m_file.open(QFile::ReadOnly)) {
        m_error = true;
        m_errorString = tr("Unable to open file.");
        return false;
    }

    return true;
}

QList<BookmarksModel::Bookmark> ChromeImporter::exportBookmarks()
{
    QList<BookmarksModel::Bookmark> list;

    QString bookmarks = m_file.readAll();
    m_file.close();

    QStringList parsedBookmarks;
    QRegExp rx("\\{(\\s*)\"date_added(.*)\"(\\s*)\\}", Qt::CaseSensitive);
    rx.setMinimal(true);

    int pos = 0;
    while ((pos = rx.indexIn(bookmarks, pos)) != -1) {
        parsedBookmarks << rx.cap(0);
        pos += rx.matchedLength();
    }

    QScriptEngine* scriptEngine = new QScriptEngine();
    foreach(QString parsedString, parsedBookmarks) {
        parsedString = "(" + parsedString + ")";
        if (scriptEngine->canEvaluate(parsedString)) {
            QScriptValue object = scriptEngine->evaluate(parsedString);
            QString name = object.property("name").toString();
            QString url = object.property("url").toString();

            if (name.isEmpty() || url.isEmpty()) {
                continue;
            }

            BookmarksModel::Bookmark b;
            b.folder = "Chrome Import";
            b.title = name;
            b.url = url;

            list.append(b);
        }
        else {
            m_error = true;
            m_errorString = tr("Cannot evaluate JSON code.");
        }
    }

    return list;
}

