#ifndef FIREFOXIMPORTER_H
#define FIREFOXIMPORTER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QFile>

#include "bookmarksmodel.h"

class FirefoxImporter : public QObject
{
    Q_OBJECT
public:
    explicit FirefoxImporter(QObject* parent = 0);

    void setFile(const QString &path);
    bool openDatabase();

    QList<BookmarksModel::Bookmark> exportBookmarks();

    bool error() { return m_error; }
    QString errorString() { return m_errorString; }

signals:

public slots:

private:
    QString m_path;
    QSqlDatabase db;

    bool m_error;
    QString m_errorString;

};

#endif // FIREFOXIMPORTER_H
