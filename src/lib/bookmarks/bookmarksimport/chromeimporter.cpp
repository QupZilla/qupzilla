/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2018 David Rosca <nowrep@gmail.com>
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

#include <QDir>
#include <QFileDialog>
#include <QVariantList>
#include <QJsonDocument>

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
#if defined(Q_OS_WIN)
    return QString("%APPDATA%/Chrome/");
#elif defined(Q_OS_OSX)
    return QDir::homePath() + QLatin1String("/Library/Application Support/Google/Chrome/");
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
    const QByteArray data = m_file.readAll();
    m_file.close();

    QJsonParseError err;
    QJsonDocument json = QJsonDocument::fromJson(data, &err);
    const QVariant res = json.toVariant();

    if (err.error != QJsonParseError::NoError || res.type() != QVariant::Map) {
        setError(BookmarksImporter::tr("Cannot parse JSON file!"));
        return 0;
    }

    QVariantMap rootMap = res.toMap().value("roots").toMap();

    BookmarkItem* root = new BookmarkItem(BookmarkItem::Folder);
    root->setTitle("Chrome Import");

    BookmarkItem* toolbar = new BookmarkItem(BookmarkItem::Folder, root);
    toolbar->setTitle(rootMap.value("bookmark_bar").toMap().value("name").toString());
    readBookmarks(rootMap.value("bookmark_bar").toMap().value("children").toList(), toolbar);

    BookmarkItem* other = new BookmarkItem(BookmarkItem::Folder, root);
    other->setTitle(rootMap.value("other").toMap().value("name").toString());
    readBookmarks(rootMap.value("other").toMap().value("children").toList(), other);

    BookmarkItem* synced = new BookmarkItem(BookmarkItem::Folder, root);
    synced->setTitle(rootMap.value("synced").toMap().value("name").toString());
    readBookmarks(rootMap.value("synced").toMap().value("synced").toList(), other);

    return root;
}

void ChromeImporter::readBookmarks(const QVariantList &list, BookmarkItem* parent)
{
    Q_ASSERT(parent);

    foreach (const QVariant &entry, list) {
        const QVariantMap map = entry.toMap();
        const QString typeString = map.value("type").toString();
        BookmarkItem::Type type;

        if (typeString == QLatin1String("url")) {
            type = BookmarkItem::Url;
        }
        else if (typeString == QLatin1String("folder")) {
            type = BookmarkItem::Folder;
        }
        else {
            continue;
        }

        BookmarkItem* item = new BookmarkItem(type, parent);
        item->setTitle(map.value("name").toString());

        if (item->isUrl()) {
            item->setUrl(QUrl::fromEncoded(map.value("url").toByteArray()));
        }

        if (map.contains("children")) {
            readBookmarks(map.value("children").toList(), item);
        }
    }
}
