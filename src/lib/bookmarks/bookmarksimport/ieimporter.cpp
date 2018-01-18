/* ============================================================
* QupZilla - Qt web browser
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
    m_path = QFileDialog::getExistingDirectory(parent, BookmarksImporter::tr("Choose file..."), standardPath());
    return m_path;
}

bool IeImporter::prepareImport()
{
    QDir dir(m_path);
    if (!dir.exists()) {
        setError(BookmarksImporter::tr("Directory does not exist."));
        return false;
    }

    return true;
}

BookmarkItem* IeImporter::importBookmarks()
{
    BookmarkItem* root = new BookmarkItem(BookmarkItem::Folder);
    root->setTitle("Internet Explorer Import");

    readDir(QDir(m_path), root);
    return root;
}

void IeImporter::readDir(const QDir &dir, BookmarkItem *parent)
{
    foreach (const QFileInfo &file, dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
        if (file.isDir()) {
            BookmarkItem* folder = new BookmarkItem(BookmarkItem::Folder, parent);
            folder->setTitle(file.baseName());

            QDir folderDir = dir;
            folderDir.cd(file.baseName());
            readDir(folderDir, folder);
        }
        else if (file.isFile()) {
            QSettings urlFile(file.absoluteFilePath(), QSettings::IniFormat);
            const QUrl url = urlFile.value("InternetShortcut/URL").toUrl();

            BookmarkItem* item = new BookmarkItem(BookmarkItem::Url, parent);
            item->setTitle(file.baseName());
            item->setUrl(url);
        }
    }
}
