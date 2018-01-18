/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018  David Rosca <nowrep@gmail.com>
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
#include "bookmarkitem.h"
#include "qzregexp.h"

#include <QUrl>
#include <QDir>
#include <QFileDialog>
#include <QTextStream>

OperaImporter::OperaImporter(QObject* parent)
    : BookmarksImporter(parent)
{
    m_stream.setCodec("UTF-8");
}

QString OperaImporter::description() const
{
    return BookmarksImporter::tr("Opera stores its bookmarks in <b>bookmarks.adr</b> text file. "
                                 "This file is usually located in");
}

QString OperaImporter::standardPath() const
{
#ifdef Q_OS_WIN
    return QString("%APPDATA%/Opera/");
#else
    return QDir::homePath() + QLatin1String("/.opera/");
#endif
}

QString OperaImporter::getPath(QWidget* parent)
{
    m_path = QFileDialog::getOpenFileName(parent, BookmarksImporter::tr("Choose file..."), standardPath(), "Bookmarks (*.adr)");
    return m_path;
}

bool OperaImporter::prepareImport()
{
    m_file.setFileName(m_path);

    if (!m_file.open(QFile::ReadOnly)) {
        setError(BookmarksImporter::tr("Unable to open file."));
        return false;
    }

    m_stream.setDevice(&m_file);

    if (m_stream.readLine() != QLatin1String("Opera Hotlist version 2.0")) {
        setError(BookmarksImporter::tr("File is not valid Opera bookmarks file!"));
        return false;
    }

    if (!m_stream.readLine().startsWith(QLatin1String("Options: encoding = utf8"))) {
        setError(BookmarksImporter::tr("Only UTF-8 encoded Opera bookmarks file is supported!"));
        return false;
    }

    return true;
}

BookmarkItem* OperaImporter::importBookmarks()
{
    BookmarkItem* root = new BookmarkItem(BookmarkItem::Folder);
    root->setTitle(QSL("Opera Import"));

    QList<BookmarkItem*> folders;
    folders.append(root);

    BookmarkItem* item = 0;

#define PARENT folders.isEmpty() ? root : folders.last()

    while (!m_stream.atEnd()) {
        switch (parseLine(m_stream.readLine())) {
        case StartFolder:
            item = new BookmarkItem(BookmarkItem::Folder, PARENT);
            while (!m_stream.atEnd()) {
                Token tok = parseLine(m_stream.readLine());
                if (tok == EmptyLine)
                    break;
                else if (tok == KeyValuePair && m_key == QLatin1String("NAME"))
                    item->setTitle(m_value);
            }
            folders.append(item);
            break;

        case EndFolder:
            if (folders.count() > 0) {
                folders.removeLast();
            }
            break;

        case StartUrl:
            item = new BookmarkItem(BookmarkItem::Url, PARENT);
            while (!m_stream.atEnd()) {
                Token tok = parseLine(m_stream.readLine());
                if (tok == EmptyLine) {
                    break;
                }
                else if (tok == KeyValuePair) {
                    if (m_key == QL1S("NAME"))
                        item->setTitle(m_value);
                    else if (m_key == QL1S("URL"))
                        item->setUrl(QUrl(m_value));
                    else if (m_key == QL1S("DESCRIPTION"))
                        item->setDescription(m_value);
                    else if (m_key == QL1S("SHORT NAME"))
                        item->setKeyword(m_value);
                }
            }
            break;

        case StartSeparator:
            item = new BookmarkItem(BookmarkItem::Separator, PARENT);
            while (!m_stream.atEnd()) {
                if (parseLine(m_stream.readLine()) == EmptyLine) {
                    break;
                }
            }
            break;

        case StartDeleted:
            while (!m_stream.atEnd()) {
                if (parseLine(m_stream.readLine()) == EmptyLine) {
                    break;
                }
            }
            break;

        default: // EmptyLine
            break;
        }
    }

#undef PARENT

    return root;
}

OperaImporter::Token OperaImporter::parseLine(const QString &line)
{
    const QString str = line.trimmed();

    if (str.isEmpty()) {
        return EmptyLine;
    }

    if (str == QLatin1String("#FOLDER")) {
        return StartFolder;
    }

    if (str == QLatin1String("-")) {
        return EndFolder;
    }

    if (str == QLatin1String("#URL")) {
        return StartUrl;
    }

    if (str == QLatin1String("#SEPERATOR")) {
        return StartSeparator;
    }

    if (str == QLatin1String("#DELETED")) {
        return StartDeleted;
    }

    int index = str.indexOf(QLatin1Char('='));

    // Let's assume "key=" is valid line with empty value (but not "=value")
    if (index > 0) {
        m_key = str.mid(0, index);
        m_value = str.mid(index + 1);
        return KeyValuePair;
    }

    return Invalid;
}
