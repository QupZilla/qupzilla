#ifndef HTMLIMPORTER_H
#define HTMLIMPORTER_H

#include <QObject>
#include <QFile>
#include <QRegExp>

#include "bookmarksmodel.h"

class HtmlImporter : public QObject
{
    Q_OBJECT
public:
    explicit HtmlImporter(QObject* parent = 0);

    void setFile(const QString &path);
    bool openFile();

    QList<BookmarksModel::Bookmark> exportBookmarks();

    bool error() { return m_error; }
    QString errorString() { return m_errorString; }

signals:

public slots:

private:
    QString m_path;
    QFile m_file;

    bool m_error;
    QString m_errorString;
};

#endif // HTMLIMPORTER_H
