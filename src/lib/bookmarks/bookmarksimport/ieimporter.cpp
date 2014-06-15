/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2013-2014  Mattias Cibien <mattias@mattiascibien.net>
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
#include "ieimporter.h"
#include "bookmarkitem.h"

#include <QDir>
#include <QUrl>
#include <QSettings>
#include <QFileDialog>

IeImporter::IeImporter(QObject* parent)
    : BookmarksImporter(parent)
{
}

QString IeImporter::description() const
{
    return BookmarksImporter::tr("Internet Explorer stores its bookmarks in <b>Favorites</b> folder. "
                                 "This folder is usually located in");
}

QString IeImporter::standardPath() const
{
    return QDir::homePath() + QLatin1String("/Favorites/");
}

QString IeImporter::getPath(QWidget* parent)
{
    m_path = QFileDialog::getOpenFileName(parent, BookmarksImporter::tr("Choose file..."), standardPath());
    return m_path;
}

bool IeImporter::prepareImport()
{
    QDir dir(m_path);
    if (!dir.exists()) {
        setError(BookmarksImporter::tr("Directory does not exist."));
        return false;
    }

    QStringList filters;
    filters << "*.url";

    urls = dir.entryInfoList(filters);

    if (urls.isEmpty()) {
        setError(BookmarksImporter::tr("The directory does not contain any bookmarks."));
        return false;
    }

    return true;
}

BookmarkItem* IeImporter::importBookmarks()
{
    BookmarkItem* root = new BookmarkItem(BookmarkItem::Folder);
    root->setTitle("Internet Explorer Import");

    foreach (QFileInfo file, urls) {
        QSettings urlFile(file.absoluteFilePath(), QSettings::IniFormat, this);

        QUrl url = urlFile.value("InternetShortcut/URL").toUrl();

        BookmarkItem* b = new BookmarkItem(BookmarkItem::Url, root);
        b->setTitle(file.baseName());
        b->setUrl(url);
    }

    return root;
}
