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
#include "adblocksubscription.h"
#include "mainapplication.h"
#include "networkmanager.h"
#include "globalfunctions.h"

#include <QFile>
#include <QTimer>
#include <QNetworkReply>
#include <QDebug>
// #define ADBLOCKSUBSCRIPTION_DEBUG

AdBlockSubscription::AdBlockSubscription(const QString &title, QObject* parent)
    : QObject(parent)
    , m_reply(0)
    , m_title(title)
{
}

QString AdBlockSubscription::title() const
{
    return m_title;
}

QString AdBlockSubscription::filePath() const
{
    return m_filePath;
}

void AdBlockSubscription::setFilePath(const QString &path)
{
    m_filePath = path;
}

QUrl AdBlockSubscription::url() const
{
    return m_url;
}

void AdBlockSubscription::setUrl(const QUrl &url)
{
    m_url = url;
}

void AdBlockSubscription::loadSubscription()
{
    QFile file(m_filePath);

    if (!file.exists()) {
        qWarning() << "AdBlockSubscription::" << __FUNCTION__ << "File does not exists" << m_filePath;
        QTimer::singleShot(0, this, SLOT(updateSubscription()));
        return;
    }

    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "AdBlockSubscription::" << __FUNCTION__ << "Unable to open adblock file for reading" << m_filePath;
        QTimer::singleShot(0, this, SLOT(updateSubscription()));
        return;
    }

    QTextStream textStream(&file);
    // Header is on 3rd line
    textStream.readLine(1024);
    textStream.readLine(1024);
    QString header = textStream.readLine(1024);

    if (!header.startsWith("[Adblock") || m_title.isEmpty()) {
        qWarning() << "AdBlockSubscription::" << __FUNCTION__ << "invalid format of adblock file" << m_filePath;
        QTimer::singleShot(0, this, SLOT(updateSubscription()));
        return;
    }

    m_rules.clear();

    while (!textStream.atEnd()) {
        const QString &line = textStream.readLine();
        m_rules.append(AdBlockRule(line, this));
    }

    populateCache();

    // Initial update
    if (m_rules.isEmpty()) {
        QTimer::singleShot(0, this, SLOT(updateSubscription()));
    }
}

void AdBlockSubscription::saveSubscription()
{
    QFile file(m_filePath);

    if (!file.open(QFile::ReadWrite | QFile::Truncate)) {
        qWarning() << "AdBlockSubscription::" << __FUNCTION__ << "Unable to open adblock file for writing:" << m_filePath;
        return;
    }

    QTextStream textStream(&file);
    textStream << "Title: " << m_title << endl;
    textStream << "Url: " << m_url.toString() << endl;
    textStream << "[Adblock Plus 1.1.1]" << endl;

    foreach(const AdBlockRule & rule, m_rules) {
        textStream << rule.filter() << endl;
    }
}

void AdBlockSubscription::updateSubscription()
{
    if (m_reply || !m_url.isValid()) {
        return;
    }

    QNetworkRequest request(m_url);
    m_reply = mApp->networkManager()->get(request);

    connect(m_reply, SIGNAL(finished()), this, SLOT(subscriptionDownloaded()));
}

void AdBlockSubscription::subscriptionDownloaded()
{
    if (m_reply != qobject_cast<QNetworkReply*>(sender())) {
        return;
    }

    QByteArray response = m_reply->readAll();

    if (m_reply->error() == QNetworkReply::NoError && !response.isEmpty()) {
        // Prepend subscription info
        response.prepend(QString("Title: %1\nUrl: %2\n").arg(title(), url().toString()).toUtf8());

        saveDownloadedData(response);

        loadSubscription();
        emit subscriptionUpdated();
    }

    m_reply->close();
    m_reply->deleteLater();
    m_reply = 0;
}

void AdBlockSubscription::saveDownloadedData(QByteArray &data)
{
    QFile file(m_filePath);

    if (!file.open(QFile::ReadWrite | QFile::Truncate)) {
        qWarning() << "AdBlockSubscription::" << __FUNCTION__ << "Unable to open adblock file for writing:" << m_filePath;
        return;
    }

    file.write(data);
    file.close();
}

const AdBlockRule* AdBlockSubscription::match(const QNetworkRequest &request, const QString &urlDomain, const QString &urlString) const
{
    foreach(const AdBlockRule * rule, m_networkExceptionRules) {
        if (rule->networkMatch(request, urlDomain, urlString)) {
            return 0;
        }
    }

    foreach(const AdBlockRule * rule, m_networkBlockRules) {
        if (rule->networkMatch(request, urlDomain, urlString)) {
            return rule;
        }
    }

    return 0;
}

QString AdBlockSubscription::elementHidingRules() const
{
    return m_elementHidingRules;
}

QString AdBlockSubscription::elementHidingRulesForDomain(const QString &domain) const
{
    QString rules;

    foreach(const AdBlockRule * rule, m_domainRestrictedCssRules) {
        if (rule->matchDomain(domain)) {
            rules.append(rule->cssSelector() + ",");
        }
    }

    return rules;
}

QList<AdBlockRule> AdBlockSubscription::allRules() const
{
    return m_rules;
}

const AdBlockRule* AdBlockSubscription::enableRule(int offset)
{
    if (!qz_listContainsIndex(m_rules, offset)) {
        return 0;
    }

    m_rules[offset].setEnabled(true);
    return &m_rules[offset];
}

const AdBlockRule* AdBlockSubscription::disableRule(int offset)
{
    if (!qz_listContainsIndex(m_rules, offset)) {
        return 0;
    }

    m_rules[offset].setEnabled(false);
    return &m_rules[offset];
}

bool AdBlockSubscription::canEditRules() const
{
    return false;
}

bool AdBlockSubscription::canBeRemoved() const
{
    return true;
}

int AdBlockSubscription::addRule(const AdBlockRule &rule)
{
    Q_UNUSED(rule)
    return -1;
}

bool AdBlockSubscription::removeRule(int offset)
{
    Q_UNUSED(offset)
    return false;
}

const AdBlockRule* AdBlockSubscription::replaceRule(const AdBlockRule &rule, int offset)
{
    Q_UNUSED(rule)
    Q_UNUSED(offset)
    return 0;
}

void AdBlockSubscription::populateCache()
{
    m_networkExceptionRules.clear();
    m_networkBlockRules.clear();
    m_domainRestrictedCssRules.clear();
    m_elementHidingRules.clear();

    for (int i = 0; i < m_rules.count(); ++i) {
        const AdBlockRule* rule = &m_rules.at(i);
        if (!rule->isEnabled()) {
            continue;
        }

        if (rule->isCssRule()) {
            if (rule->isDomainRestricted()) {
                m_domainRestrictedCssRules.append(rule);
            }
            else {
                m_elementHidingRules.append(rule->cssSelector() + ",");
            }
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

// AdBlockEasyList

AdBlockEasyList::AdBlockEasyList(QObject* parent)
    : AdBlockSubscription(tr("EasyList"), parent)
{
    setUrl(QUrl("https://easylist-downloads.adblockplus.org/easylist.txt"));
    setFilePath(mApp->currentProfilePath() + "adblock/easylist.txt");
}

bool AdBlockEasyList::canBeRemoved() const
{
    return false;
}

void AdBlockEasyList::saveDownloadedData(QByteArray &data)
{
    QFile file(filePath());

    if (!file.open(QFile::ReadWrite | QFile::Truncate)) {
        qWarning() << "AdBlockSubscription::" << __FUNCTION__ << "Unable to open adblock file for writing:" << filePath();
        return;
    }

    // Third-party advertisers rules are with start domain (||) placeholder which needs regexps
    // So we are ignoring it for keeping good performance
    data = data.left(data.indexOf("!-----------------------------Third-party adverts-----------------------------!"));

    file.write(data);
    file.close();
}

// AdBlockCustomList

AdBlockCustomList::AdBlockCustomList(QObject* parent)
    : AdBlockSubscription(tr("Custom Rules"), parent)
{
    setFilePath(mApp->currentProfilePath() + "adblock/customlist.txt");
}

bool AdBlockCustomList::canEditRules() const
{
    return true;
}

bool AdBlockCustomList::canBeRemoved() const
{
    return false;
}

int AdBlockCustomList::addRule(const AdBlockRule &rule)
{
    m_rules.append(rule);
    populateCache();

    return m_rules.count() - 1;
}

bool AdBlockCustomList::removeRule(int offset)
{
    if (!qz_listContainsIndex(m_rules, offset)) {
        return false;
    }

    m_rules.removeAt(offset);
    populateCache();

    return true;
}

const AdBlockRule* AdBlockCustomList::replaceRule(const AdBlockRule &rule, int offset)
{
    if (!qz_listContainsIndex(m_rules, offset)) {
        return 0;
    }

    m_rules[offset] = rule;
    populateCache();

    return &m_rules[offset];
}
