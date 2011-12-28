/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca <nowrep@gmail.com>
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
#include "adblocksubscription.h"
#include "mainapplication.h"
#include "networkmanager.h"
// #define ADBLOCKSUBSCRIPTION_DEBUG

AdBlockSubscription::AdBlockSubscription(QObject* parent)
    : QObject(parent)
    , m_downloading(0)
{
    loadRules();
}

void AdBlockSubscription::loadRules()
{
    QString fileName = mApp->getActiveProfilPath() + "adblocklist.txt";

    QFile file(fileName);
    if (file.exists()) {
        if (!file.open(QFile::ReadOnly)) {
            qWarning() << "AdBlockSubscription::" << __FUNCTION__ << "Unable to open adblock file for reading" << fileName;
        }
        else {
            QTextStream textStream(&file);
            QString header = textStream.readLine(1024);
            if (!header.startsWith("[Adblock")) {
                qWarning() << "AdBlockSubscription::" << __FUNCTION__ << "adblock file does not start with [Adblock" << fileName << "Header:" << header;
                file.close();
                file.remove();
            }
            else {
                m_rules.clear();
                while (!textStream.atEnd()) {
                    QString line = textStream.readLine();
                    m_rules.append(AdBlockRule(line));
                }
                populateCache();
                emit rulesChanged();
            }
        }
    }

    if (m_rules.isEmpty()) {
        // Initial update
        QTimer::singleShot(0, this, SLOT(updateNow()));
    }
}

void AdBlockSubscription::scheduleUpdate()
{
    QTimer::singleShot(1000 * 30, this, SLOT(updateNow()));
}

void AdBlockSubscription::updateNow()
{
    if (m_downloading) {
        return;
    }

    QNetworkRequest request(QUrl("https://easylist-downloads.adblockplus.org/easylist.txt"));
    QNetworkReply* reply = mApp->networkManager()->get(request);
    m_downloading = reply;
    connect(reply, SIGNAL(finished()), this, SLOT(rulesDownloaded()));
}

void AdBlockSubscription::rulesDownloaded()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }

    QByteArray response = reply->readAll();
    reply->close();
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        return;
    }

    if (response.isEmpty()) {
        return;
    }

    QString fileName = mApp->getActiveProfilPath() + "adblocklist.txt";
    QFile file(fileName);
    if (!file.open(QFile::ReadWrite | QFile::Truncate)) {
        qWarning() << "AdBlockSubscription::" << __FUNCTION__ << "Unable to open adblock file for writing:" << fileName;
        return;
    }

    response = response.left(response.indexOf("General element hiding rules"));

    bool customRules = false;
    foreach(const AdBlockRule & rule, allRules()) {
        if (rule.filter().contains("*******- user custom filters")) {
            customRules = true;
            response.append("! *******- user custom filters -*************\n");
            continue;
        }
        if (!customRules) {
            continue;
        }
        response.append(rule.filter() + "\n");
    }

    file.write(response);
    file.close();
    loadRules();
    emit rulesUpdated();
    m_downloading = 0;
}

void AdBlockSubscription::saveRules()
{
    QString fileName = mApp->getActiveProfilPath() + "adblocklist.txt";

    QFile file(fileName);
    if (!file.open(QFile::ReadWrite | QFile::Truncate)) {
        qWarning() << "AdBlockSubscription::" << __FUNCTION__ << "Unable to open adblock file for writing:" << fileName;
        return;
    }

    QTextStream textStream(&file);
    textStream << "[Adblock Plus 1.1.1]" << endl;

    foreach(const AdBlockRule & rule, m_rules) {
        textStream << rule.filter() << endl;
    }
}

const AdBlockRule* AdBlockSubscription::allow(const QString &urlString) const
{
    foreach(const AdBlockRule * rule, m_networkExceptionRules) {
        if (rule->networkMatch(urlString)) {
            return rule;
        }
    }
    return 0;
}

const AdBlockRule* AdBlockSubscription::block(const QString &urlString) const
{
    foreach(const AdBlockRule * rule, m_networkBlockRules) {
        if (rule->networkMatch(urlString)) {
            return rule;
        }
    }
    return 0;
}

QList<AdBlockRule> AdBlockSubscription::allRules() const
{
    return m_rules;
}

int AdBlockSubscription::addRule(const AdBlockRule &rule)
{
    m_rules.append(rule);
    populateCache();
    emit rulesChanged();
    return m_rules.count() - 1;
}

void AdBlockSubscription::removeRule(int offset)
{
    if (offset < 0 || offset >= m_rules.count()) {
        return;
    }
    m_rules.removeAt(offset);
    populateCache();
    emit rulesChanged();
}

void AdBlockSubscription::replaceRule(const AdBlockRule &rule, int offset)
{
    if (offset < 0 || offset >= m_rules.count()) {
        return;
    }
    m_rules[offset] = rule;
    populateCache();
    emit rulesChanged();
}

void AdBlockSubscription::populateCache()
{
    m_networkExceptionRules.clear();
    m_networkBlockRules.clear();
    m_pageRules.clear();

    for (int i = 0; i < m_rules.count(); ++i) {
        const AdBlockRule* rule = &m_rules.at(i);
        if (!rule->isEnabled()) {
            continue;
        }

        if (rule->isCSSRule()) {
            m_pageRules.append(rule);
            continue;
        }

        if (rule->isException()) {
            m_networkExceptionRules.append(rule);
        }
        else {
            m_networkBlockRules.append(rule);
        }
    }
}

