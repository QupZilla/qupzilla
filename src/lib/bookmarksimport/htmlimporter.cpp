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

int qzMin(int a, int b)
{
    if (a > -1 && b > -1) {
        return qMin(a, b);
    }

    if (a > -1) {
        return a;
    }
    else {
        return b;
    }
}

QList<BookmarksModel::Bookmark> HtmlImporter::exportBookmarks()
{
    QList<BookmarksModel::Bookmark> list;

    QString bookmarks = QString::fromUtf8(m_file.readAll());
    m_file.close();

    // Converting tags to lower case -,-
    // For some reason Qt::CaseInsensitive is not everytime insensitive :-D

    bookmarks.replace(QLatin1String("<DL"), QLatin1String("<dl"));
    bookmarks.replace(QLatin1String("</DL"), QLatin1String("</dl"));
    bookmarks.replace(QLatin1String("<DT"), QLatin1String("<dt"));
    bookmarks.replace(QLatin1String("</DT"), QLatin1String("</dt"));
    bookmarks.replace(QLatin1String("<P"), QLatin1String("<p"));
    bookmarks.replace(QLatin1String("</P"), QLatin1String("</p"));
    bookmarks.replace(QLatin1String("<A"), QLatin1String("<a"));
    bookmarks.replace(QLatin1String("</A"), QLatin1String("</a"));
    bookmarks.replace(QLatin1String("HREF="), QLatin1String("href="));
    bookmarks.replace(QLatin1String("<H3"), QLatin1String("<h3"));
    bookmarks.replace(QLatin1String("</H3"), QLatin1String("</h3"));

    bookmarks = bookmarks.left(bookmarks.lastIndexOf(QLatin1String("</dl><p>")));
    int start = bookmarks.indexOf(QLatin1String("<dl><p>"));

    QStringList folders("Html Import");

    while (start > 0) {
        QString string = bookmarks.mid(start);

        int posOfFolder = string.indexOf(QLatin1String("<dt><h3"));
        int posOfEndFolder = string.indexOf(QLatin1String("</dl><p>"));
        int posOfLink = string.indexOf(QLatin1String("<dt><a"));

        int nearest = qzMin(posOfLink, qzMin(posOfFolder, posOfEndFolder));
        if (nearest == -1) {
            break;
        }

        if (nearest == posOfFolder) {
            // Next is folder
            QRegExp rx("<dt><h3(.*)>(.*)</h3>");
            rx.setMinimal(true);
            rx.indexIn(string);

//            QString arguments = rx.cap(1);
            QString folderName = rx.cap(2).trimmed();

            folders.append(folderName);

            start += posOfFolder + rx.cap(0).size();
        }
        else if (nearest == posOfEndFolder) {
            // Next is end of folder
            if (!folders.isEmpty()) {
                folders.removeLast();
            }

            start += posOfEndFolder + 8;
        }
        else {
            // Next is link
            QRegExp rx("<dt><a(.*)>(.*)</a>");
            rx.setMinimal(true);
            rx.indexIn(string);

            QString arguments = rx.cap(1);
            QString linkName = rx.cap(2).trimmed();

            QRegExp rx2("href=\"(.*)\"");
            rx2.setMinimal(true);
            rx2.indexIn(arguments);

            QUrl url = QUrl::fromEncoded(rx2.cap(1).trimmed().toUtf8());

            start += posOfLink + rx.cap(0).size();

            if (linkName.isEmpty() || url.isEmpty() || url.scheme() == QLatin1String("place")
                    || url.scheme() == QLatin1String("about")) {
                continue;
            }

            BookmarksModel::Bookmark b;
            b.folder = folders.last();
            b.title = linkName;
            b.url = url;

            list.append(b);
        }
    }

    return list;
}
