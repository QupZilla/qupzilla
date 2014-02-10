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
#include "operaimporter.h"
#include "bookmarksimportdialog.h"
#include "bookmarkitem.h"
#include "qzregexp.h"

#include <QUrl>

OperaImporter::OperaImporter(QObject* parent)
    : QObject(parent)
    , m_error(false)
    , m_errorString(BookmarksImportDialog::tr("No Error"))
{
}

void OperaImporter::setFile(const QString &path)
{
    m_path = path;
}

bool OperaImporter::openFile()
{
    m_file.setFileName(m_path);

    if (!m_file.open(QFile::ReadOnly)) {
        m_error = true;
        m_errorString = BookmarksImportDialog::tr("Unable to open file.");
        return false;
    }

    return true;
}

BookmarkItem* OperaImporter::exportBookmarks()
{
    QString bookmarks = QString::fromUtf8(m_file.readAll());
    m_file.close();

    QzRegExp rx("#URL(.*)CREATED", Qt::CaseSensitive);
    rx.setMinimal(true);

    BookmarkItem* root = new BookmarkItem(BookmarkItem::Folder);
    root->setTitle("Opera Import");

    int pos = 0;
    while ((pos = rx.indexIn(bookmarks, pos)) != -1) {
        QString string = rx.cap(1);
        pos += rx.matchedLength();

        QzRegExp rx2("NAME=(.*)\\n");
        rx2.setMinimal(true);
        rx2.indexIn(string);
        QString name = rx2.cap(1).trimmed();

        rx2.setPattern("URL=(.*)\\n");
        rx2.indexIn(string);
        QUrl url = QUrl::fromEncoded(rx2.cap(1).trimmed().toUtf8());

        if (name.isEmpty() || url.isEmpty()) {
            continue;
        }

        BookmarkItem* b = new BookmarkItem(BookmarkItem::Url, root);
        b->setTitle(name);
        b->setUrl(url);
    }

    return root;
}
