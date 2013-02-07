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

#ifndef ADBLOCKRULE_H
#define ADBLOCKRULE_H

#include <QObject>
#include <QRegExp>
#include <QStringList>

#include "qz_namespace.h"

class QNetworkRequest;
class QUrl;

class AdBlockSubscription;

class AdBlockRule
{
public:
    AdBlockRule(const QString &filter = QString(), AdBlockSubscription* subscription = 0);

    AdBlockSubscription* subscription() const;
    void setSubscription(AdBlockSubscription* subscription);

    QString filter() const;
    void setFilter(const QString &filter);

    bool isCssRule() const;
    QString cssSelector() const;

    bool isDocument() const;
    bool isElemhide() const;

    bool isDomainRestricted() const;
    bool isException() const;

    bool isComment() const;
    bool isEnabled() const;
    void setEnabled(bool enabled);

    bool isSlow() const;
    bool isInternalDisabled() const;

    bool networkMatch(const QNetworkRequest &request, const QString &domain, const QString &encodedUrl) const;
    bool urlMatch(const QUrl &url) const;

    bool matchDomain(const QString &domain) const;
    bool matchThirdParty(const QNetworkRequest &request) const;
    bool matchObject(const QNetworkRequest &request) const;
    bool matchSubdocument(const QNetworkRequest &request) const;
    bool matchXmlHttpRequest(const QNetworkRequest &request) const;
    bool matchImage(const QString &encodedUrl) const;

private:
    void parseFilter();
    void parseDomains(const QString &domains, const QChar &separator);

    bool _matchDomain(const QString &domain, const QString &filter) const;

    AdBlockSubscription* m_subscription;
    QString m_filter;

    bool m_enabled;
    bool m_cssRule;
    bool m_exception;

    bool m_internalDisabled;
    bool m_domainRestricted;

    bool m_useRegExp;
    QRegExp m_regExp;

    bool m_useDomainMatch;
    bool m_useEndsMatch;

    QString m_cssSelector;
    QString m_matchString;

    // Rule $options
    QStringList m_allowedDomains;
    QStringList m_blockedDomains;

    bool m_thirdParty;
    bool m_thirdPartyException;

    bool m_object;
    bool m_objectException;

    bool m_subdocument;
    bool m_subdocumentException;

    bool m_xmlhttprequest;
    bool m_xmlhttprequestException;

    bool m_image;
    bool m_imageException;

    // Exception only options
    bool m_document;
    bool m_elemhide;

    Qt::CaseSensitivity m_caseSensitivity;
};

#endif // ADBLOCKRULE_H

