#include "operaimporter.h"

#include <QDebug>

OperaImporter::OperaImporter(QObject* parent)
    : QObject(parent)
    , m_error(false)
    , m_errorString(tr("No Error"))
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
        m_errorString = tr("Unable to open file.");
        return false;
    }

    return true;
}

QList<BookmarksModel::Bookmark> OperaImporter::exportBookmarks()
{
    QList<BookmarksModel::Bookmark> list;

    QString bookmarks = m_file.readAll();
    m_file.close();

    QRegExp rx("#URL(.*)CREATED", Qt::CaseSensitive);
    rx.setMinimal(true);

    int pos = 0;
    while ((pos = rx.indexIn(bookmarks, pos)) != -1) {
        QString string = rx.cap(1);
        pos += rx.matchedLength();

        QRegExp rx2("NAME=(.*)\\n");
        rx2.setMinimal(true);
        rx2.indexIn(string);
        QString name = rx2.cap(1);

        rx2.setPattern("URL=(.*)\\n");
        rx2.indexIn(string);
        QString url = rx2.cap(1);

        if (name.isEmpty() || url.isEmpty())
            continue;

        BookmarksModel::Bookmark b;
        b.folder = "Opera Import";
        b.title = name;
        b.url = url;

        list.append(b);
    }

    return list;
}
