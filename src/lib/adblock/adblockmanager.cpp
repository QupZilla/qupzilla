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
#include "adblockmanager.h"
#include "adblockdialog.h"
#include "adblockpage.h"
#include "adblocksubscription.h"
#include "adblockblockednetworkreply.h"
#include "mainapplication.h"
#include "webpage.h"
#include "networkmanager.h"
#include "qupzilla.h"
#include "settings.h"

#include <QDateTime>

AdBlockManager* AdBlockManager::s_adBlockManager = 0;

AdBlockManager::AdBlockManager(QObject* parent)
    : QObject(parent)
    , m_loaded(false)
    , m_enabled(true)
    , m_adBlockNetwork(0)
    , m_adBlockPage(0)
{
}

AdBlockManager* AdBlockManager::instance()
{
    if (!s_adBlockManager) {
        s_adBlockManager = new AdBlockManager(mApp->networkManager());
    }

    return s_adBlockManager;
}

void AdBlockManager::setEnabled(bool enabled)
{
    if (isEnabled() == enabled) {
        return;
    }
    m_enabled = enabled;
    emit rulesChanged();
    mApp->sendMessages(Qz::AM_SetAdBlockIconEnabled, enabled);
}

AdBlockPage* AdBlockManager::page()
{
    if (!m_adBlockPage) {
        m_adBlockPage = new AdBlockPage(this);
    }
    return m_adBlockPage;
}

AdBlockSubscription* AdBlockManager::subscription()
{
    return m_subscriptions.at(0);
}

QNetworkReply* AdBlockManager::block(const QNetworkRequest &request)
{
    const QString &urlString = request.url().toEncoded();
    const QString &urlScheme = request.url().scheme();

    if (!isEnabled() || urlScheme == "data" || urlScheme == "qrc" || urlScheme == "file" || urlScheme == "qupzilla") {
        return 0;
    }

    const AdBlockRule* blockedRule = 0;

    foreach(AdBlockSubscription * subscription, m_subscriptions) {
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

            AdBlockBlockedNetworkReply* reply = new AdBlockBlockedNetworkReply(subscription, blockedRule, this);
            reply->setRequest(request);

            return reply;
        }
    }

    return 0;
}

void AdBlockManager::load()
{
    if (m_loaded) {
        return;
    }
    m_loaded = true;

    Settings settings;
    settings.beginGroup("AdBlock");
    m_enabled = settings.value("enabled", m_enabled).toBool();
    QDateTime lastUpdate = settings.value("lastUpdate", QDateTime()).toDateTime();
    settings.endGroup();

    AdBlockSubscription* easyList = new AdBlockSubscription();
    easyList->setTitle("EasyList");
    connect(easyList, SIGNAL(rulesChanged()), this, SIGNAL(rulesChanged()));
    connect(easyList, SIGNAL(rulesUpdated()), this, SLOT(rulesUpdated()));

    m_subscriptions.append(easyList);

    if (lastUpdate.addDays(3) < QDateTime::currentDateTime()) {
        easyList->scheduleUpdate();
    }
}

void AdBlockManager::rulesUpdated()
{
    Settings settings;
    settings.beginGroup("AdBlock");
    settings.setValue("lastUpdate", QDateTime::currentDateTime());
    settings.endGroup();

    emit rulesChanged();
}

void AdBlockManager::save()
{
    if (!m_loaded) {
        return;
    }

    subscription()->saveRules();

    Settings settings;
    settings.beginGroup(QLatin1String("AdBlock"));
    settings.setValue(QLatin1String("enabled"), m_enabled);
    settings.endGroup();
}

AdBlockDialog* AdBlockManager::showDialog()
{
    if (!m_adBlockDialog) {
        m_adBlockDialog = new AdBlockDialog(mApp->getWindow());
    }

    m_adBlockDialog.data()->show();
    return m_adBlockDialog.data();
}

void AdBlockManager::showRule()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        showDialog()->search->setText(action->data().toString());
    }
}

AdBlockManager::~AdBlockManager()
{
}
