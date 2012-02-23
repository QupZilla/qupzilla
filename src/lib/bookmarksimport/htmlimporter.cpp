#include "htmlimporter.h"
#include "bookmarksimportdialog.h"

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
