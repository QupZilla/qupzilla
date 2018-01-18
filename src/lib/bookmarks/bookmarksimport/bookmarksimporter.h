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
#ifndef BOOKMARKSIMPORTER_H
#define BOOKMARKSIMPORTER_H

#include <QObject>

#include "qzcommon.h"

class QIcon;

class BookmarkItem;

class QUPZILLA_EXPORT BookmarksImporter : public QObject
{
    Q_OBJECT

public:
    explicit BookmarksImporter(QObject* parent = 0);
    virtual ~BookmarksImporter();

    bool error() const;
    QString errorString() const;

    virtual QString description() const = 0;
    virtual QString standardPath() const = 0;

    // Get filename from user (or a directory)
    virtual QString getPath(QWidget* parent) = 0;

    // Prepare import (check if file exists, ...), return false on error
    virtual bool prepareImport() = 0;

    // Import bookmarks (it must return root folder)
    virtual BookmarkItem* importBookmarks() = 0;

protected:
    // Empty error = no error
    void setError(const QString &error);

private:
    QString m_error;
};

#endif // BOOKMARKSIMPORTER_H
