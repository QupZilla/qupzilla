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

#ifndef ADBLOCKSUBSCRIPTION_H
#define ADBLOCKSUBSCRIPTION_H

#include <QVarLengthArray>
#include <QList>
#include <QUrl>

#include "qz_namespace.h"
#include "adblockrule.h"

class QNetworkRequest;
class QNetworkReply;
class QUrl;

class FollowRedirectReply;

class QT_QUPZILLA_EXPORT AdBlockSubscription : public QObject
{
    Q_OBJECT
public:
    explicit AdBlockSubscription(const QString &title, QObject* parent = 0);

    QString title() const;

    QString filePath() const;
    void setFilePath(const QString &path);

    QUrl url() const;
    void setUrl(const QUrl &url);

    virtual void loadSubscription(const QStringList &disabledRules);
    virtual void saveSubscription();

    const AdBlockRule* match(const QNetworkRequest &request, const QString &urlDomain, const QString &urlString) const;

    bool adBlockDisabledForUrl(const QUrl &url) const;
    bool elemHideDisabledForUrl(const QUrl &url) const;

    QString elementHidingRules() const;
    QString elementHidingRulesForDomain(const QString &domain) const;

    const AdBlockRule* rule(int offset) const;
    QList<AdBlockRule> allRules() const;

    const AdBlockRule* enableRule(int offset);
    const AdBlockRule* disableRule(int offset);

    virtual bool canEditRules() const;
    virtual bool canBeRemoved() const;

    virtual int addRule(const AdBlockRule &rule);
    virtual bool removeRule(int offset);
    virtual const AdBlockRule* replaceRule(const AdBlockRule &rule, int offset);

public slots:
    void updateSubscription();

signals:
    void subscriptionUpdated();

protected slots:
    void subscriptionDownloaded();

protected:
    virtual void saveDownloadedData(QByteArray &data);

    void populateCache();

    FollowRedirectReply* m_reply;

    QList<AdBlockRule> m_rules;
    QString m_elementHidingRules;

    QVarLengthArray<const AdBlockRule*> m_networkExceptionRules;
    QVarLengthArray<const AdBlockRule*> m_networkBlockRules;
    QVarLengthArray<const AdBlockRule*> m_domainRestrictedCssRules;

    QVarLengthArray<const AdBlockRule*> m_documentRules;
    QVarLengthArray<const AdBlockRule*> m_elemhideRules;

private:
    QString m_title;
    QString m_filePath;

    QUrl m_url;
    bool m_updated;
};

class AdBlockEasyList : public AdBlockSubscription
{
    Q_OBJECT
public:
    explicit AdBlockEasyList(QObject* parent = 0);

    bool canBeRemoved() const;

protected:
    void saveDownloadedData(QByteArray &data);
};

class AdBlockCustomList : public AdBlockSubscription
{
    Q_OBJECT
public:
    explicit AdBlockCustomList(QObject* parent = 0);

    void saveSubscription();

    bool canEditRules() const;
    bool canBeRemoved() const;

    bool containsFilter(const QString &filter) const;
    bool removeFilter(const QString &filter);

    int addRule(const AdBlockRule &rule);
    bool removeRule(int offset);
    const AdBlockRule* replaceRule(const AdBlockRule &rule, int offset);

signals:
    void subscriptionEdited();
};

#endif // ADBLOCKSUBSCRIPTION_H

