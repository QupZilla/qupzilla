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
#include "adblockschemehandler.h"
#include "adblockmanager.h"
#include "emptynetworkreply.h"

#include <QNetworkRequest>
#include <QMessageBox>

#include <QUrlQuery>

AdBlockSchemeHandler::AdBlockSchemeHandler()
    : SchemeHandler()
{
}

QNetworkReply* AdBlockSchemeHandler::createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice* outgoingData)
{
    Q_UNUSED(outgoingData)

    if (op != QNetworkAccessManager::GetOperation) {
        return 0;
    }

    const QUrl url = request.url();
    const QList<QPair<QString, QString> > queryItems = QUrlQuery(url).queryItems();

    QString subscriptionTitle;
    QString subscriptionUrl;

    for (int i = 0; i < queryItems.count(); ++i) {
        QPair<QString, QString> pair = queryItems.at(i);
        if (pair.first == QLatin1String("location")) {
            subscriptionUrl = pair.second;
        }
        else if (pair.first == QLatin1String("title")) {
            subscriptionTitle = pair.second;
        }
    }

    if (subscriptionTitle.isEmpty() || subscriptionUrl.isEmpty()) {
        return 0;
    }

    const QString message = AdBlockManager::tr("Do you want to add <b>%1</b> subscription?").arg(subscriptionTitle);

    QMessageBox::StandardButton result = QMessageBox::question(0, AdBlockManager::tr("AdBlock Subscription"), message, QMessageBox::Yes | QMessageBox::No);
    if (result == QMessageBox::Yes) {
        AdBlockManager::instance()->addSubscription(subscriptionTitle, subscriptionUrl);
        AdBlockManager::instance()->showDialog();
    }

    return new EmptyNetworkReply;
}
