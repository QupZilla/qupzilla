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
 * Copyright (c) 2009, Zsombor Gegesy <gzsombor@gmail.com>
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

#include "adblockrule.h"
#include "adblocksubscription.h"

#include <QDebug>
#include <QRegExp>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QNetworkRequest>
#include <QWebFrame>
#include <QWebPage>

// Version for Qt < 4.8 has one issue, it will wrongly
// count .co.uk (and others) as second-level domain
static QString toSecondLevelDomain(const QUrl &url)
{
#if QT_VERSION >= 0x040800
    const QString &topLevelDomain = url.topLevelDomain();
    const QString &urlHost = url.host();

    if (topLevelDomain.isEmpty() || urlHost.isEmpty()) {
        return QString();
    }

    QString domain = urlHost.left(urlHost.size() - topLevelDomain.size());

    if (domain.count(QLatin1Char('.')) == 0) {
        return urlHost;
    }

    while (domain.count(QLatin1Char('.')) != 0) {
        domain = domain.mid(domain.indexOf(QLatin1Char('.')) + 1);
    }

    return domain + topLevelDomain;
#else
    QString domain = url.host();

    if (domain.count(QLatin1Char('.')) == 0) {
        return QString();
    }

    while (domain.count(QLatin1Char('.')) != 1) {
        domain = domain.mid(domain.indexOf(QLatin1Char('.')) + 1);
    }

    return domain;
#endif
}

AdBlockRule::AdBlockRule(const QString &filter, AdBlockSubscription* subscription)
    : m_subscription(subscription)
    , m_enabled(true)
    , m_cssRule(false)
    , m_exception(false)
    , m_internalDisabled(false)
    , m_domainRestricted(false)
    , m_useRegExp(false)
    , m_useDomainMatch(false)
    , m_useEndsMatch(false)
    , m_thirdParty(false)
    , m_thirdPartyException(false)
    , m_object(false)
    , m_objectException(false)
    , m_subdocument(false)
    , m_subdocumentException(false)
    , m_xmlhttprequest(false)
    , m_xmlhttprequestException(false)
    , m_image(false)
    , m_imageException(false)
    , m_document(false)
    , m_elemhide(false)
    , m_caseSensitivity(Qt::CaseInsensitive)
{
    setFilter(filter);
}

AdBlockSubscription* AdBlockRule::subscription() const
{
    return m_subscription;
}

void AdBlockRule::setSubscription(AdBlockSubscription* subscription)
{
    m_subscription = subscription;
}

QString AdBlockRule::filter() const
{
    return m_filter;
}

void AdBlockRule::setFilter(const QString &filter)
{
    m_filter = filter;
    parseFilter();
}

bool AdBlockRule::isCssRule() const
{
    return m_cssRule;
}

QString AdBlockRule::cssSelector() const
{
    return m_cssSelector;
}

bool AdBlockRule::isDocument() const
{
    return m_document;
}

bool AdBlockRule::isElemhide() const
{
    return m_elemhide;
}

bool AdBlockRule::isDomainRestricted() const
{
    return m_domainRestricted;
}

bool AdBlockRule::isException() const
{
    return m_exception;
}

bool AdBlockRule::isComment() const
{
    return m_filter.startsWith(QLatin1Char('!'));
}

bool AdBlockRule::isEnabled() const
{
    return m_enabled;
}

void AdBlockRule::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

bool AdBlockRule::isSlow() const
{
    return m_useRegExp;
}

bool AdBlockRule::isInternalDisabled() const
{
    return m_internalDisabled;
}

bool AdBlockRule::networkMatch(const QNetworkRequest &request, const QString &domain, const QString &encodedUrl) const
{
    if (m_cssRule || !m_enabled || m_internalDisabled) {
        return false;
    }

    bool matched = false;

    if (m_useDomainMatch) {
        matched = _matchDomain(domain, m_matchString);
    }
    else if (m_useEndsMatch) {
        matched = encodedUrl.endsWith(m_matchString, m_caseSensitivity);
    }
    else if (m_useRegExp) {
        matched = (m_regExp.indexIn(encodedUrl) != -1);
    }
    else {
        matched = encodedUrl.contains(m_matchString, m_caseSensitivity);
    }

    if (matched) {
        // Check domain restrictions
        if (m_domainRestricted && !matchDomain(domain)) {
            return false;
        }

        // Check third-party restriction
        if (m_thirdParty && !matchThirdParty(request)) {
            return false;
        }

        // Check object restrictions
        if (m_object && !matchObject(request)) {
            return false;
        }

        // Check subdocument restriction
        if (m_subdocument && !matchSubdocument(request)) {
            return false;
        }

        // Check xmlhttprequest restriction
        if (m_xmlhttprequest && !matchXmlHttpRequest(request)) {
            return false;
        }

        // Check image restriction
        if (m_image && !matchImage(encodedUrl)) {
            return false;
        }
    }

    return matched;
}

bool AdBlockRule::urlMatch(const QUrl &url) const
{
    if (!m_document && !m_elemhide) {
        return false;
    }

    const QString &encodedUrl = url.toEncoded();
    const QString &domain = url.host();

    return networkMatch(QNetworkRequest(url), domain, encodedUrl);
}

bool AdBlockRule::matchDomain(const QString &domain) const
{
    if (!m_enabled) {
        return false;
    }

    if (!m_domainRestricted) {
        return true;
    }

    if (m_blockedDomains.isEmpty()) {
        foreach(const QString & d, m_allowedDomains) {
            if (_matchDomain(domain, d)) {
                return true;
            }
        }
    }
    else if (m_allowedDomains.isEmpty()) {
        foreach(const QString & d, m_blockedDomains) {
            if (_matchDomain(domain, d)) {
                return false;
            }
        }
        return true;
    }
    else {
        foreach(const QString & d, m_blockedDomains) {
            if (_matchDomain(domain, d)) {
                return false;
            }
        }

        foreach(const QString & d, m_allowedDomains) {
            if (_matchDomain(domain, d)) {
                return true;
            }
        }
    }

    return false;
}

bool AdBlockRule::matchThirdParty(const QNetworkRequest &request) const
{
    const QString &referer = request.attribute(QNetworkRequest::Attribute(QNetworkRequest::User + 151), QString()).toString();

    if (referer.isEmpty()) {
        return false;
    }

    // Third-party matching should be performed on second-level domains
    const QString &refererHost = toSecondLevelDomain(QUrl(referer));
    const QString &host = toSecondLevelDomain(request.url());

    bool match = refererHost != host;

    return m_thirdPartyException ? !match : match;
}

bool AdBlockRule::matchObject(const QNetworkRequest &request) const
{
    bool match = request.attribute(QNetworkRequest::Attribute(QNetworkRequest::User + 150)).toString() == QLatin1String("object");

    return m_objectException ? !match : match;
}

bool AdBlockRule::matchSubdocument(const QNetworkRequest &request) const
{
    QWebFrame* originatingFrame = static_cast<QWebFrame*>(request.originatingObject());
    if (!originatingFrame) {
        return false;
    }

    QWebPage* page = originatingFrame->page();
    if (!page) {
        return false;
    }

    bool match = !(originatingFrame == page->mainFrame());

    return m_subdocumentException ? !match : match;
}

bool AdBlockRule::matchXmlHttpRequest(const QNetworkRequest &request) const
{
    bool match = request.rawHeader("X-Requested-With") == QByteArray("XMLHttpRequest");

    return m_xmlhttprequestException ? !match : match;
}

bool AdBlockRule::matchImage(const QString &encodedUrl) const
{
    bool match = encodedUrl.endsWith(QLatin1String(".png")) ||
                 encodedUrl.endsWith(QLatin1String(".jpg")) ||
                 encodedUrl.endsWith(QLatin1String(".gif")) ||
                 encodedUrl.endsWith(QLatin1String(".jpeg"));

    return m_imageException ? !match : match;
}

void AdBlockRule::parseFilter()
{
    QString parsedLine = m_filter;

    // Empty rule or just comment
    if (m_filter.trimmed().isEmpty() || m_filter.startsWith(QLatin1Char('!'))) {
        m_enabled = false;
        return;
    }

    // CSS Element hiding rule
    if (parsedLine.contains(QLatin1String("##"))) {
        m_cssRule = true;
        int pos = parsedLine.indexOf(QLatin1String("##"));

        // Domain restricted rule
        if (!parsedLine.startsWith(QLatin1String("##"))) {
            QString domains = parsedLine.left(pos);
            parseDomains(domains, QLatin1Char(','));
        }

        m_cssSelector = parsedLine.mid(pos + 2);
        m_cssSelector.remove('\\');
        // CSS rule cannot have more options -> stop parsing
        return;
    }

    // Exception always starts with @@
    if (parsedLine.startsWith(QLatin1String("@@"))) {
        m_exception = true;
        parsedLine = parsedLine.mid(2);
    }

    // Parse all options following $ char
    int optionsIndex = parsedLine.indexOf(QLatin1Char('$'));
    if (optionsIndex >= 0) {
        QStringList options = parsedLine.mid(optionsIndex + 1).split(QLatin1Char(','));

        int handledOptions = 0;
        foreach(const QString & option, options) {
            if (option.startsWith(QLatin1String("domain="))) {
                parseDomains(option.mid(7), QLatin1Char('|'));
                ++handledOptions;
            }
            else if (option == QLatin1String("match-case")) {
                m_caseSensitivity = Qt::CaseSensitive;
                ++handledOptions;
            }
            else if (option.endsWith(QLatin1String("third-party"))) {
                m_thirdParty = true;
                m_thirdPartyException = option.startsWith(QLatin1Char('~'));
                ++handledOptions;
            }
            else if (option.endsWith(QLatin1String("object"))) {
                m_object = true;
                m_objectException = option.startsWith(QLatin1Char('~'));
                ++handledOptions;
            }
            else if (option.endsWith(QLatin1String("subdocument"))) {
                m_subdocument = true;
                m_subdocumentException = option.startsWith(QLatin1Char('~'));
                ++handledOptions;
            }
            else if (option.endsWith(QLatin1String("xmlhttprequest"))) {
                m_xmlhttprequest = true;
                m_xmlhttprequestException = option.startsWith(QLatin1Char('~'));
                ++handledOptions;
            }
            else if (option.endsWith(QLatin1String("image"))) {
                m_image = true;
                m_imageException = option.startsWith(QLatin1Char('~'));
                ++handledOptions;
            }
            else if (option == QLatin1String("document") && m_exception) {
                m_document = true;
                ++handledOptions;
            }
            else if (option == QLatin1String("elemhide") && m_exception) {
                m_elemhide = true;
                ++handledOptions;
            }
            else if (option == QLatin1String("collapse")) {
                // Hiding placeholders of blocked elements
                ++handledOptions;
            }
        }

        // If we don't handle all options, it's safer to just disable this rule
        if (handledOptions != options.count()) {
            m_internalDisabled = true;
            return;
        }

        parsedLine = parsedLine.left(optionsIndex);
    }

    // Rule is classic regexp
    if (parsedLine.startsWith(QLatin1Char('/')) && parsedLine.endsWith(QLatin1Char('/'))) {
        parsedLine = parsedLine.mid(1);
        parsedLine = parsedLine.left(parsedLine.size() - 1);

        m_useRegExp = true;
        m_regExp = QRegExp(parsedLine, m_caseSensitivity, QRegExp::RegExp);
        return;
    }

    // Remove starting and ending wildcards (*)
    if (parsedLine.startsWith(QLatin1Char('*'))) {
        parsedLine = parsedLine.mid(1);
    }

    if (parsedLine.endsWith(QLatin1Char('*'))) {
        parsedLine = parsedLine.left(parsedLine.size() - 1);
    }

    // We can use fast string matching for domain here
    if (parsedLine.startsWith(QLatin1String("||")) && parsedLine.endsWith(QLatin1Char('^'))
            && !parsedLine.contains(QRegExp("[/:?=&\\*]"))) {
        parsedLine = parsedLine.mid(2);
        parsedLine = parsedLine.left(parsedLine.size() - 1);

        m_useDomainMatch = true;
        m_matchString = parsedLine;
        return;
    }

    // If rule contains only | at end, we can also use string matching
    if (parsedLine.endsWith(QLatin1Char('|')) && !parsedLine.contains(QRegExp("[\\^\\*]"))
            && parsedLine.count(QLatin1Char('|')) == 1) {
        parsedLine = parsedLine.left(parsedLine.size() - 1);

        m_useEndsMatch = true;
        m_matchString = parsedLine;
        return;
    }

    // If we still find a wildcard (*) or separator (^) or (|)
    // we must modify parsedLine to comply with QRegExp
    if (parsedLine.contains(QLatin1Char('*')) || parsedLine.contains(QLatin1Char('^'))
            || parsedLine.contains(QLatin1Char('|'))) {
        parsedLine.replace(QRegExp(QLatin1String("\\*+")), QLatin1String("*"))       // remove multiple wildcards
        .replace(QRegExp(QLatin1String("\\^\\|$")), QLatin1String("^"))    // remove anchors following separator placeholder
        .replace(QRegExp(QLatin1String("^(\\*)")), QString())      // remove leading wildcards
        .replace(QRegExp(QLatin1String("(\\*)$")), QString())
        .replace(QRegExp(QLatin1String("(\\W)")), QLatin1String("\\\\1"))  // escape special symbols
        .replace(QRegExp(QLatin1String("^\\\\\\|\\\\\\|")),
                 QLatin1String("^[\\w\\-]+:\\/+(?!\\/)(?:[^\\/]+\\.)?"))   // process extended anchor at expression start
        .replace(QRegExp(QLatin1String("\\\\\\^")),
                 QLatin1String("(?:[^\\w\\d\\-.%]|$)"))                    // process separator placeholders
        .replace(QRegExp(QLatin1String("^\\\\\\|")), QLatin1String("^"))   // process anchor at expression start
        .replace(QRegExp(QLatin1String("\\\\\\|$")), QLatin1String("$"))   // process anchor at expression end
        .replace(QRegExp(QLatin1String("\\\\\\*")), QLatin1String(".*"));  // replace wildcards by .*

        m_useRegExp = true;
        m_regExp = QRegExp(parsedLine, m_caseSensitivity, QRegExp::RegExp);
        return;
    }

    // We haven't found anything that needs use of regexp, yay!
    m_useRegExp = false;
    m_matchString = parsedLine;
}

void AdBlockRule::parseDomains(const QString &domains, const QChar &separator)
{
    QStringList domainsList = domains.split(separator);

    foreach(const QString domain, domainsList) {
        if (domain.isEmpty()) {
            continue;
        }
        if (domain.startsWith(QLatin1Char('~'))) {
            m_blockedDomains.append(domain.mid(1));
        }
        else {
            m_allowedDomains.append(domain);
        }
    }

    m_domainRestricted = (!m_blockedDomains.isEmpty() || !m_allowedDomains.isEmpty());
}

bool AdBlockRule::_matchDomain(const QString &domain, const QString &filter) const
{
    if (!domain.endsWith(filter)) {
        return false;
    }

    int index = domain.indexOf(filter);

    if (index == 0 || filter[0] == QLatin1Char('.')) {
        return true;
    }

    return domain[index - 1] == QLatin1Char('.');
}
