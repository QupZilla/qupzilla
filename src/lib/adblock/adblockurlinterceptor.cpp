/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2015-2018 David Rosca <nowrep@gmail.com>
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

#include "adblockurlinterceptor.h"
#include "adblockrule.h"
#include "qztools.h"

#include <QUrlQuery>

AdBlockUrlInterceptor::AdBlockUrlInterceptor(AdBlockManager *manager)
    : UrlInterceptor(manager)
    , m_manager(manager)
{
}

void AdBlockUrlInterceptor::interceptRequest(QWebEngineUrlRequestInfo &request)
{
    QString ruleFilter;
    QString ruleSubscription;
    if (!m_manager->block(request, ruleFilter, ruleSubscription)) {
        return;
    }

    if (request.resourceType() == QWebEngineUrlRequestInfo::ResourceTypeMainFrame) {
        QString page;
        page.append(QzTools::readAllFileContents(QSL(":adblock/data/adblock.html")));
        page.replace(QSL("%FAVICON%"), QSL("qrc:adblock/data/adblock_big.png"));
        page.replace(QSL("%IMAGE%"), QSL("qrc:adblock/data/adblock_big.png"));
        page.replace(QSL("%TITLE%"), tr("Blocked content"));
        page.replace(QSL("%RULE%"), tr("Blocked by <i>%1 (%2)</i>").arg(ruleFilter, ruleSubscription));
        page = QzTools::applyDirectionToPage(page);
        request.redirect(QUrl(QString::fromUtf8(QByteArray("data:text/html;base64,") + page.toUtf8().toBase64())));
    } else {
        request.block(true);
    }

    AdBlockedRequest r;
    r.requestUrl = request.requestUrl();
    r.firstPartyUrl = request.firstPartyUrl();
    r.requestMethod = request.requestMethod();
    r.resourceType = request.resourceType();
    r.navigationType = request.navigationType();
    r.rule = ruleFilter;
    emit requestBlocked(r);
}
