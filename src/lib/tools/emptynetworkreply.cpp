/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2018  David Rosca <nowrep@gmail.com>
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
#include "emptynetworkreply.h"

#include <QTimer>

EmptyNetworkReply::EmptyNetworkReply(QObject* parent)
    : QNetworkReply(parent)
{
    setOperation(QNetworkAccessManager::GetOperation);
    setError(QNetworkReply::OperationCanceledError, QSL("QupZilla:No Error"));

    open(QIODevice::ReadOnly);

    QTimer::singleShot(0, this, SLOT(delayedFinish()));
}

void EmptyNetworkReply::delayedFinish()
{
    emit finished();
}

qint64 EmptyNetworkReply::readData(char* data, qint64 maxSize)
{
    Q_UNUSED(data)
    Q_UNUSED(maxSize)
    return 0;
}
