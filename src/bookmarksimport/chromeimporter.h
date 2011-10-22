#ifndef CHROMEIMPORTER_H
#define CHROMEIMPORTER_H

#include <QObject>
#include <QFile>
#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>
#include <QRegExp>

#include "bookmarksmodel.h"

class ChromeImporter : public QObject
{
    Q_OBJECT
public:
    explicit ChromeImporter(QObject* parent = 0);

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

#endif // CHROMEIMPORTER_H
