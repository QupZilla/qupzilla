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
#include "htmlimporter.h"
#include "bookmarksimportdialog.h"

#include <QRegExp>

HtmlImporter::HtmlImporter(QObject* parent)
    : QObject(parent)
    , m_error(false)
    , m_errorString(BookmarksImportDialog::tr("No Error"))
{
}

void HtmlImporter::setFile(const QString &path)
{
    m_path = path;
}

bool HtmlImporter::openFile()
{
    m_file.setFileName(m_path);

    if (!m_file.open(QFile::ReadOnly)) {
        m_error = true;
        m_errorString = BookmarksImportDialog::tr("Unable to open file.");
        return false;
    }

    return true;
}

QList<BookmarksModel::Bookmark> HtmlImporter::exportBookmarks()
{
    QList<BookmarksModel::Bookmark> list;

    QString bookmarks = m_file.readAll();
    m_file.close();

    QRegExp rx("<a (.*)</a>", Qt::CaseInsensitive);
    rx.setMinimal(true);

    int pos = 0;
    while ((pos = rx.indexIn(bookmarks, pos)) != -1) {
        QString string = rx.cap(0);
        pos += rx.matchedLength();

        QRegExp rx2(">(.*)</a>", Qt::CaseInsensitive);
        rx2.setMinimal(true);
        rx2.indexIn(string);
        QString name = rx2.cap(1);

        rx2.setPattern("href=\"(.*)\"");
        rx2.indexIn(string);
        QUrl url = QUrl::fromEncoded(rx2.cap(1).toUtf8());

        if (name.isEmpty() || url.isEmpty() || url.scheme() == "place" || url.scheme() == "about") {
            continue;
        }

        BookmarksModel::Bookmark b;
        b.folder = "Html Import";
        b.title = name;
        b.url = url;

        list.append(b);
    }

    return list;
}
