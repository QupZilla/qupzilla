/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2014  David Rosca <nowrep@gmail.com>
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
#ifndef BOOKMARKSEXPORTER_H
#define BOOKMARKSEXPORTER_H

#include <QObject>

#include "qzcommon.h"

class BookmarkItem;

class QUPZILLA_EXPORT BookmarksExporter : public QObject
{
    Q_OBJECT

public:
    explicit BookmarksExporter(QObject* parent = 0);
    virtual ~BookmarksExporter();

    bool error() const;
    QString errorString() const;

    virtual QString name() const = 0;

    // Get filename from user (or a directory)
    virtual QString getPath(QWidget* parent) = 0;

    // Export bookmarks, return false on error
    virtual bool exportBookmarks(BookmarkItem* root) = 0;

protected:
    // Empty error = no error
    void setError(const QString &error);

private:
    QString m_error;
};

#endif // BOOKMARKSEXPORTER_H
