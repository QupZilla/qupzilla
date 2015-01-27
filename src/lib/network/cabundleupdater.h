/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
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
#ifndef CABUNDLEUPDATER_H
#define CABUNDLEUPDATER_H

#if QTWEBENGINE_DISABLED

#include <QObject>

#include "qzcommon.h"

class QNetworkReply;

class NetworkManager;

class QUPZILLA_EXPORT CaBundleUpdater : public QObject
{
    Q_OBJECT
public:
    explicit CaBundleUpdater(NetworkManager* manager, QObject* parent = 0);

signals:

public slots:

private slots:
    void start();
    void replyFinished();

private:
    enum Progress { Start, CheckLastUpdate, LoadBundle };

    NetworkManager* m_manager;
    Progress m_progress;
    QNetworkReply* m_reply;

    QString m_bundleVersionFileName;
    QString m_bundleFileName;
    QString m_lastUpdateFileName;

    int m_latestBundleVersion;
};

#endif

#endif // CABUNDLEUPDATER_H
