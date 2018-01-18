/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2013-2014  David Rosca <nowrep@gmail.com>
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
#include "delayedfilewatcher.h"

#include <QTimer>

DelayedFileWatcher::DelayedFileWatcher(QObject* parent)
    : QFileSystemWatcher(parent)
{
    init();
}

DelayedFileWatcher::DelayedFileWatcher(const QStringList &paths, QObject* parent)
    : QFileSystemWatcher(paths, parent)
{
    init();
}

void DelayedFileWatcher::init()
{
    connect(this, SIGNAL(directoryChanged(QString)), this, SLOT(slotDirectoryChanged(QString)));
    connect(this, SIGNAL(fileChanged(QString)), this, SLOT(slotFileChanged(QString)));
}

void DelayedFileWatcher::slotDirectoryChanged(const QString &path)
{
    m_dirQueue.enqueue(path);
    QTimer::singleShot(500, this, SLOT(dequeueDirectory()));
}

void DelayedFileWatcher::slotFileChanged(const QString &path)
{
    m_fileQueue.enqueue(path);
    QTimer::singleShot(500, this, SLOT(dequeueFile()));
}

void DelayedFileWatcher::dequeueDirectory()
{
    emit delayedDirectoryChanged(m_dirQueue.dequeue());
}

void DelayedFileWatcher::dequeueFile()
{
    emit delayedFileChanged(m_fileQueue.dequeue());
}
