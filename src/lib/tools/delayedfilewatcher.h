/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2013  David Rosca <nowrep@gmail.com>
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
#ifndef DELAYEDFILEWATCHER_H
#define DELAYEDFILEWATCHER_H

#include <QFileSystemWatcher>
#include <QPointer>
#include <QQueue>

#include "qz_namespace.h"

class QT_QUPZILLA_EXPORT DelayedFileWatcher : public QFileSystemWatcher
{
    Q_OBJECT

public:
    explicit DelayedFileWatcher(QObject* parent = 0);
    explicit DelayedFileWatcher(const QStringList &paths, QObject* parent = 0);

signals:
    void delayedDirectoryChanged(const QString &path);
    void delayedFileChanged(const QString &path);

private slots:
    void slotDirectoryChanged(const QString &path);
    void slotFileChanged(const QString &path);

    void dequeueDirectory();
    void dequeueFile();

private:
    void init();

    QQueue<QString> m_dirQueue;
    QQueue<QString> m_fileQueue;
};

#endif // DELAYEDFILEWATCHER_H
