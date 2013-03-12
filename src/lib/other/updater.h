/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2013  David Rosca <nowrep@gmail.com>
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
#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>

#include "qz_namespace.h"

class QNetworkReply;
class QUrl;

class QupZilla;

class QT_QUPZILLA_EXPORT Updater : public QObject
{
    Q_OBJECT
public:
    explicit Updater(QupZilla* mainClass, QObject* parent = 0);
    ~Updater();

    struct Version {
        bool isValid;
        int majorVersion;
        int minorVersion;
        int revisionNumber;

        bool operator<(const Version &other) const {
            if (this->majorVersion < other.majorVersion)
                return true;
            if (this->minorVersion < other.minorVersion)
                return true;
            if (this->revisionNumber < other.revisionNumber)
                return true;

            return false;
        }

        bool operator>(const Version &other) const {
            return !operator<(other);
        }

        bool operator==(const Version &other) const {
            return (this->majorVersion == other.majorVersion &&
                    this->minorVersion == other.minorVersion &&
                    this->revisionNumber == other.revisionNumber);
        }

        bool operator>=(const Version &other) const {
            if (*this == other)
                return true;
            return *this > other;
        }

        bool operator<=(const Version &other) const {
            if (*this == other)
                return true;
            return *this < other;
        }
    };

    static Version parseVersionFromString(const QString &string);

private slots:
    void downCompleted(QNetworkReply* reply);
    void start();
    void downloadNewVersion();

private:
    void startDownloadingUpdateInfo(const QUrl &url);

    QupZilla* p_QupZilla;
};

#endif // UPDATER_H
