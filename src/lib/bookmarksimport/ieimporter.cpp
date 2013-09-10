#include "ieimporter.h"

#include "bookmarksimportdialog.h"

#include <QDir>
#include <QSettings>

IeImporter::IeImporter(QObject *parent) :
    QObject(parent)
  , m_error(false)
  , m_errorString(BookmarksImportDialog::tr("No Error"))
{
}


void IeImporter::setFile(const QString &path)
{
    m_path = path;
}

bool IeImporter::openFile()
{
    QDir dir(m_path);
    if(!dir.exists()) {
        m_error = true;
        m_errorString = BookmarksImportDialog::tr("Directory does not exists.");
        return false;
    }


    QStringList filters;
    filters << "*.url";

    urls = dir.entryInfoList(filters);

    if(urls.isEmpty()) {
        m_error = true;
        m_errorString = BookmarksImportDialog::tr("The directory does not contain any bookmarks.");
        return false;
    }

    return true;
}


QVector<BookmarksModel::Bookmark> IeImporter::exportBookmarks()
{
    QVector<BookmarksModel::Bookmark> bookmarks;

    foreach (QFileInfo file, urls) {
        QSettings urlFile(file.absoluteFilePath(), QSettings::IniFormat, this);

        QUrl url = urlFile.value("InternetShortcut/URL").toUrl();

        BookmarksModel::Bookmark bookmark;
        bookmark.folder = "Internet Explorer Import";
        bookmark.title = file.baseName();
        bookmark.url = url;

        bookmarks.append(bookmark);
    }

    return bookmarks;
}
