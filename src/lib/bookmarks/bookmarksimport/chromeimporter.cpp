/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
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
#include "bookmarkitem.h"
#include "qzregexp.h"

#include <QDir>
#include <QFileDialog>
#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>

ChromeImporter::ChromeImporter(QObject* parent)
    : BookmarksImporter(parent)
{
}

QString ChromeImporter::description() const
{
    return BookmarksImporter::tr("Google Chrome stores its bookmarks in <b>Bookmarks</b> text file. "
                                 "This file is usually located in");
}

QString ChromeImporter::standardPath() const
{
#ifdef Q_OS_WIN
    return QString("%APPDATA%/Chrome/");
#else
    return QDir::homePath() + QLatin1String("/.config/chrome/");
#endif
}

QString ChromeImporter::getPath(QWidget* parent)
{
    m_path = QFileDialog::getOpenFileName(parent, BookmarksImporter::tr("Choose file..."), standardPath(), "Bookmarks (Bookmarks)");
    return m_path;
}

bool ChromeImporter::prepareImport()
{
    m_file.setFileName(m_path);

    if (!m_file.open(QFile::ReadOnly)) {
        setError(BookmarksImporter::tr("Unable to open file."));
        return false;
    }

    return true;
}

BookmarkItem* ChromeImporter::importBookmarks()
{
    QString bookmarks = QString::fromUtf8(m_file.readAll());
    m_file.close();

    QStringList parsedBookmarks;
    QzRegExp rx("\\{(\\s*)\"date_added(.*)\"(\\s*)\\}", Qt::CaseSensitive);
    rx.setMinimal(true);

    int pos = 0;
    while ((pos = rx.indexIn(bookmarks, pos)) != -1) {
        parsedBookmarks << rx.cap(0);
        pos += rx.matchedLength();
    }

    BookmarkItem* root = new BookmarkItem(BookmarkItem::Folder);
    root->setTitle("Chrome Import");

    QScriptEngine* scriptEngine = new QScriptEngine();
    foreach (QString parsedString, parsedBookmarks) {
        parsedString = "(" + parsedString + ")";
        if (scriptEngine->canEvaluate(parsedString)) {
            QScriptValue object = scriptEngine->evaluate(parsedString);
            QString name = object.property("name").toString().trimmed();
            QUrl url = QUrl::fromEncoded(object.property("url").toString().trimmed().toUtf8());

            if (name.isEmpty() || url.isEmpty()) {
                continue;
            }

            BookmarkItem* b = new BookmarkItem(BookmarkItem::Url, root);
            b->setTitle(name);
            b->setUrl(url);
        }
        else {
            m_error = true;
            m_errorString = BookmarksImporter::tr("Cannot evaluate JSON code.");
        }
    }

    return root;

}
