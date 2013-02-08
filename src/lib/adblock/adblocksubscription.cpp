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
#include "adblockmanager.h"
#include "mainapplication.h"
#include "networkmanager.h"
#include "qztools.h"
#include "followredirectreply.h"

#include <QFile>
#include <QTimer>
#include <QNetworkReply>
#include <QDebug>

AdBlockSubscription::AdBlockSubscription(const QString &title, QObject* parent)
    : QObject(parent)
    , m_reply(0)
    , m_title(title)
    , m_updated(false)
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

void AdBlockSubscription::loadSubscription(const QStringList &disabledRules)
{
    QFile file(m_filePath);

    if (!file.exists()) {
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

    if (!header.startsWith(QLatin1String("[Adblock")) || m_title.isEmpty()) {
        qWarning() << "AdBlockSubscription::" << __FUNCTION__ << "invalid format of adblock file" << m_filePath;
        QTimer::singleShot(0, this, SLOT(updateSubscription()));
        return;
    }

    m_rules.clear();

    while (!textStream.atEnd()) {
        AdBlockRule rule(textStream.readLine(), this);

        if (disabledRules.contains(rule.filter())) {
            rule.setEnabled(false);
        }

        m_rules.append(rule);
    }

    populateCache();

    // Initial update
    if (m_rules.isEmpty() && !m_updated) {
        QTimer::singleShot(0, this, SLOT(updateSubscription()));
    }
}

void AdBlockSubscription::saveSubscription()
{
}

void AdBlockSubscription::updateSubscription()
{
    if (m_reply || !m_url.isValid()) {
        return;
    }

    m_reply = new FollowRedirectReply(m_url, mApp->networkManager());

    connect(m_reply, SIGNAL(finished()), this, SLOT(subscriptionDownloaded()));
}

void AdBlockSubscription::subscriptionDownloaded()
{
    if (m_reply != qobject_cast<FollowRedirectReply*>(sender())) {
        return;
    }

    QByteArray response = QString::fromUtf8(m_reply->readAll()).toUtf8();

    if (m_reply->error() == QNetworkReply::NoError && response.startsWith("[Adblock")) {
        // Prepend subscription info
        response.prepend(QString("Title: %1\nUrl: %2\n").arg(title(), url().toString()).toUtf8());

        saveDownloadedData(response);

        loadSubscription(AdBlockManager::instance()->disabledRules());
        emit subscriptionUpdated();
    }

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
    int count = m_networkExceptionRules.count();
    for (int i = 0; i < count; ++i) {
        const AdBlockRule* rule = m_networkExceptionRules.at(i);
        if (rule->networkMatch(request, urlDomain, urlString)) {
            return 0;
        }
    }

    count = m_networkBlockRules.count();
    for (int i = 0; i < count; ++i) {
        const AdBlockRule* rule = m_networkBlockRules.at(i);
        if (rule->networkMatch(request, urlDomain, urlString)) {
            return rule;
        }
    }

    return 0;
}

bool AdBlockSubscription::adBlockDisabledForUrl(const QUrl &url) const
{
    int count = m_documentRules.count();
    for (int i = 0; i < count; ++i) {
        const AdBlockRule* rule = m_documentRules.at(i);
        if (rule->urlMatch(url)) {
            return true;
        }
    }

    return false;
}

bool AdBlockSubscription::elemHideDisabledForUrl(const QUrl &url) const
{
    if (adBlockDisabledForUrl(url)) {
        return true;
    }

    int count = m_elemhideRules.count();
    for (int i = 0; i < count; ++i) {
        const AdBlockRule* rule = m_elemhideRules.at(i);
        if (rule->urlMatch(url)) {
            return true;
        }
    }

    return false;
}

QString AdBlockSubscription::elementHidingRules() const
{
    return m_elementHidingRules;
}

QString AdBlockSubscription::elementHidingRulesForDomain(const QString &domain) const
{
    QString rules;

    int count = m_domainRestrictedCssRules.count();
    for (int i = 0; i < count; ++i) {
        const AdBlockRule* rule = m_domainRestrictedCssRules.at(i);
        if (rule->matchDomain(domain)) {
            rules.append(rule->cssSelector() + QLatin1Char(','));
        }
    }

    return rules;
}

const AdBlockRule* AdBlockSubscription::rule(int offset) const
{
    if (!QzTools::listContainsIndex(m_rules, offset)) {
        return 0;
    }

    return &m_rules[offset];
}

QList<AdBlockRule> AdBlockSubscription::allRules() const
{
    return m_rules;
}

const AdBlockRule* AdBlockSubscription::enableRule(int offset)
{
    if (!QzTools::listContainsIndex(m_rules, offset)) {
        return 0;
    }

    AdBlockRule* rule = &m_rules[offset];
    rule->setEnabled(true);
    AdBlockManager::instance()->removeDisabledRule(rule->filter());

    if (rule->isCssRule()) {
        populateCache();
        mApp->reloadUserStyleSheet();
    }

    return rule;
}

const AdBlockRule* AdBlockSubscription::disableRule(int offset)
{
    if (!QzTools::listContainsIndex(m_rules, offset)) {
        return 0;
    }

    AdBlockRule* rule = &m_rules[offset];
    rule->setEnabled(false);
    AdBlockManager::instance()->addDisabledRule(rule->filter());

    if (rule->isCssRule()) {
        populateCache();
        mApp->reloadUserStyleSheet();
    }

    return rule;
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
    m_documentRules.clear();
    m_elemhideRules.clear();

    int count = m_rules.count();
    for (int i = 0; i < count; ++i) {
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
        }
        else if (rule->isDocument()) {
            m_documentRules.append(rule);
        }
        else if (rule->isElemhide()) {
            m_elemhideRules.append(rule);
        }
        else if (rule->isException()) {
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
    data = data.left(data.indexOf(QLatin1String("!-----------------------------Third-party adverts-----------------------------!")));

    file.write(data);
    file.close();
}

// AdBlockCustomList

AdBlockCustomList::AdBlockCustomList(QObject* parent)
    : AdBlockSubscription(tr("Custom Rules"), parent)
{
    setFilePath(mApp->currentProfilePath() + "adblock/customlist.txt");
}

void AdBlockCustomList::saveSubscription()
{
    QFile file(filePath());

    if (!file.open(QFile::ReadWrite | QFile::Truncate)) {
        qWarning() << "AdBlockSubscription::" << __FUNCTION__ << "Unable to open adblock file for writing:" << filePath();
        return;
    }

    QTextStream textStream(&file);
    textStream << "Title: " << title() << endl;
    textStream << "Url: " << url().toString() << endl;
    textStream << "[Adblock Plus 1.1.1]" << endl;

    foreach(const AdBlockRule & rule, m_rules) {
        textStream << rule.filter() << endl;
    }

    file.close();
}

bool AdBlockCustomList::canEditRules() const
{
    return true;
}

bool AdBlockCustomList::canBeRemoved() const
{
    return false;
}

bool AdBlockCustomList::containsFilter(const QString &filter) const
{
    foreach(const AdBlockRule & rule, m_rules) {
        if (rule.filter() == filter) {
            return true;
        }
    }

    return false;
}

bool AdBlockCustomList::removeFilter(const QString &filter)
{
    for (int i = 0; i < m_rules.count(); ++i) {
        const AdBlockRule rule = m_rules.at(i);

        if (rule.filter() == filter) {
            return removeRule(i);
        }
    }

    return false;
}

int AdBlockCustomList::addRule(const AdBlockRule &rule)
{
    m_rules.append(rule);
    populateCache();

    emit subscriptionEdited();

    return m_rules.count() - 1;
}

bool AdBlockCustomList::removeRule(int offset)
{
    if (!QzTools::listContainsIndex(m_rules, offset)) {
        return false;
    }

    const QString &filter = m_rules[offset].filter();

    m_rules.removeAt(offset);
    populateCache();

    emit subscriptionEdited();

    AdBlockManager::instance()->removeDisabledRule(filter);

    return true;
}

const AdBlockRule* AdBlockCustomList::replaceRule(const AdBlockRule &rule, int offset)
{
    if (!QzTools::listContainsIndex(m_rules, offset)) {
        return 0;
    }

    m_rules[offset] = rule;
    populateCache();

    emit subscriptionEdited();

    return &m_rules[offset];
}
