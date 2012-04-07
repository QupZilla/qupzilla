/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
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
/**
 * Copyright (c) 2009, Benjamin C. Meyer <ben@meyerhome.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Benjamin Meyer nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "adblocknetwork.h"
#include "adblockblockednetworkreply.h"
#include "adblockmanager.h"
#include "adblocksubscription.h"
#include "mainapplication.h"
#include "webpage.h"

AdBlockNetwork::AdBlockNetwork(QObject* parent)
    : QObject(parent)
{
}

QNetworkReply* AdBlockNetwork::block(const QNetworkRequest &request)
{
    const QString &urlString = request.url().toEncoded();
    const QString &urlScheme = request.url().scheme();

    if (urlScheme == "data" || urlScheme == "qrc" || urlScheme == "file" || urlScheme == "qupzilla") {
        return 0;
    }

    AdBlockManager* manager = AdBlockManager::instance();
    if (!manager->isEnabled()) {
        return 0;
    }

    const AdBlockRule* blockedRule = 0;
    AdBlockSubscription* subscription = manager->subscription();

    if (subscription->allow(urlString)) {
        return 0;
    }

    if (const AdBlockRule* rule = subscription->block(urlString)) {
        blockedRule = rule;
    }

    if (blockedRule) {
        QVariant v = request.attribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 100));
        WebPage* webPage = static_cast<WebPage*>(v.value<void*>());
        if (WebPage::isPointerSafeToUse(webPage)) {
            webPage->addAdBlockRule(blockedRule->filter(), request.url());
        }

        AdBlockBlockedNetworkReply* reply = new AdBlockBlockedNetworkReply(request, blockedRule, this);
        return reply;
    }
    return 0;
}
