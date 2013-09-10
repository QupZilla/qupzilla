#ifndef IEIMPORTER_H
#define IEIMPORTER_H

#include <QObject>

#include "qz_namespace.h"
#include "bookmarksmodel.h"

#include <QFileInfoList>

class IeImporter : public QObject
{
    Q_OBJECT
public:
    explicit IeImporter(QObject *parent = 0);

    void setFile(const QString &path);
    bool openFile();

    QVector<BookmarksModel::Bookmark> exportBookmarks();

    bool error() { return m_error; }
    QString errorString() { return m_errorString; }


private:
    bool m_error;
    QString m_errorString;
    QString m_path;

    QFileInfoList urls;
};

#endif // IEIMPORTER_H
