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
#include "qztools.h"
#include "qzregexp.h"

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
    const QString topLevelDomain = url.topLevelDomain();
    const QString urlHost = url.host();

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
    , m_type(StringContainsMatchRule)
    , m_caseSensitivity(Qt::CaseInsensitive)
    , m_isEnabled(true)
    , m_isException(false)
    , m_isInternalDisabled(false)
    , m_regExp(0)
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
    return m_type == CssRule;
}

QString AdBlockRule::cssSelector() const
{
    return m_matchString;
}

bool AdBlockRule::isDocument() const
{
    return hasOption(DocumentOption);
}

bool AdBlockRule::isElemhide() const
{
    return hasOption(ElementHideOption);
}

bool AdBlockRule::isDomainRestricted() const
{
    return hasOption(DomainRestrictedOption);
}

bool AdBlockRule::isException() const
{
    return m_isException;
}

bool AdBlockRule::isComment() const
{
    return m_filter.startsWith(QLatin1Char('!'));
}

bool AdBlockRule::isEnabled() const
{
    return m_isEnabled;
}

void AdBlockRule::setEnabled(bool enabled)
{
    m_isEnabled = enabled;
}

bool AdBlockRule::isSlow() const
{
    return m_regExp != 0;
}

bool AdBlockRule::isInternalDisabled() const
{
    return m_isInternalDisabled;
}

bool AdBlockRule::urlMatch(const QUrl &url) const
{
    if (!hasOption(DocumentOption) && !hasOption(ElementHideOption)) {
        return false;
    }

    const QString encodedUrl = url.toEncoded();
    const QString domain = url.host();

    return networkMatch(QNetworkRequest(url), domain, encodedUrl);
}

bool AdBlockRule::networkMatch(const QNetworkRequest &request, const QString &domain, const QString &encodedUrl) const
{
    if (m_type == CssRule || !m_isEnabled || m_isInternalDisabled) {
        return false;
    }

    bool matched = false;

    if (m_type == StringContainsMatchRule) {
        matched = encodedUrl.contains(m_matchString, m_caseSensitivity);
    }
    else if (m_type == DomainMatchRule) {
        matched = isMatchingDomain(domain, m_matchString);
    }
    else if (m_type == StringEndsMatchRule) {
        matched = encodedUrl.endsWith(m_matchString, m_caseSensitivity);
    }
    else if (m_type == RegExpMatchRule) {
        if (!isMatchingRegExpStrings(encodedUrl)) {
            return false;
        }

        matched = (m_regExp->regExp.indexIn(encodedUrl) != -1);
    }

    if (matched) {
        // Check domain restrictions
        if (hasOption(DomainRestrictedOption) && !matchDomain(domain)) {
            return false;
        }

        // Check third-party restriction
        if (hasOption(ThirdPartyOption) && !matchThirdParty(request)) {
            return false;
        }

        // Check object restrictions
        if (hasOption(ObjectOption) && !matchObject(request)) {
            return false;
        }

        // Check subdocument restriction
        if (hasOption(SubdocumentOption) && !matchSubdocument(request)) {
            return false;
        }

        // Check xmlhttprequest restriction
        if (hasOption(XMLHttpRequestOption) && !matchXmlHttpRequest(request)) {
            return false;
        }

        // Check image restriction
        if (hasOption(ImageOption) && !matchImage(encodedUrl)) {
            return false;
        }
    }

    return matched;
}

bool AdBlockRule::matchDomain(const QString &domain) const
{
    if (!m_isEnabled) {
        return false;
    }

    if (!hasOption(DomainRestrictedOption)) {
        return true;
    }

    if (m_blockedDomains.isEmpty()) {
        foreach (const QString &d, m_allowedDomains) {
            if (isMatchingDomain(domain, d)) {
                return true;
            }
        }
    }
    else if (m_allowedDomains.isEmpty()) {
        foreach (const QString &d, m_blockedDomains) {
            if (isMatchingDomain(domain, d)) {
                return false;
            }
        }
        return true;
    }
    else {
        foreach (const QString &d, m_blockedDomains) {
            if (isMatchingDomain(domain, d)) {
                return false;
            }
        }

        foreach (const QString &d, m_allowedDomains) {
            if (isMatchingDomain(domain, d)) {
                return true;
            }
        }
    }

    return false;
}

bool AdBlockRule::matchThirdParty(const QNetworkRequest &request) const
{
    const QString referer = request.attribute(QNetworkRequest::Attribute(QNetworkRequest::User + 151), QString()).toString();

    if (referer.isEmpty()) {
        return false;
    }

    // Third-party matching should be performed on second-level domains
    const QString refererHost = toSecondLevelDomain(QUrl(referer));
    const QString host = toSecondLevelDomain(request.url());

    bool match = refererHost != host;

    return hasException(ThirdPartyOption) ? !match : match;
}

bool AdBlockRule::matchObject(const QNetworkRequest &request) const
{
    bool match = request.attribute(QNetworkRequest::Attribute(QNetworkRequest::User + 150)).toString() == QLatin1String("object");

    return hasException(ObjectOption) ? !match : match;
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

    return hasException(SubdocumentOption) ? !match : match;
}

bool AdBlockRule::matchXmlHttpRequest(const QNetworkRequest &request) const
{
    bool match = request.rawHeader("X-Requested-With") == QByteArray("XMLHttpRequest");

    return hasException(XMLHttpRequestOption) ? !match : match;
}

bool AdBlockRule::matchImage(const QString &encodedUrl) const
{
    bool match = encodedUrl.endsWith(QLatin1String(".png")) ||
                 encodedUrl.endsWith(QLatin1String(".jpg")) ||
                 encodedUrl.endsWith(QLatin1String(".gif")) ||
                 encodedUrl.endsWith(QLatin1String(".jpeg"));

    return hasException(ImageOption) ? !match : match;
}

void AdBlockRule::parseFilter()
{
    QString parsedLine = m_filter;

    // Empty rule or just comment
    if (m_filter.trimmed().isEmpty() || m_filter.startsWith(QLatin1Char('!'))) {
        // We want to differentiate rule disabled by user and rule disabled in subscription file
        // m_isInternalDisabled is also used when rule is disabled due to all options not being supported
        m_isEnabled = false;
        m_isInternalDisabled = true;
        m_type = Invalid;
        return;
    }

    // CSS Element hiding rule
    if (parsedLine.contains(QLatin1String("##"))) {
        m_type = CssRule;
        int pos = parsedLine.indexOf(QLatin1String("##"));

        // Domain restricted rule
        if (!parsedLine.startsWith(QLatin1String("##"))) {
            QString domains = parsedLine.left(pos);
            parseDomains(domains, QLatin1Char(','));
        }

        m_matchString = parsedLine.mid(pos + 2);

        // CSS rule cannot have more options -> stop parsing
        return;
    }

    // Exception always starts with @@
    if (parsedLine.startsWith(QLatin1String("@@"))) {
        m_isException = true;
        parsedLine = parsedLine.mid(2);
    }

    // Parse all options following $ char
    int optionsIndex = parsedLine.indexOf(QLatin1Char('$'));
    if (optionsIndex >= 0) {
        QStringList options = parsedLine.mid(optionsIndex + 1).split(QLatin1Char(','), QString::SkipEmptyParts);

        int handledOptions = 0;
        foreach (const QString &option, options) {
            if (option.startsWith(QLatin1String("domain="))) {
                parseDomains(option.mid(7), QLatin1Char('|'));
                ++handledOptions;
            }
            else if (option == QLatin1String("match-case")) {
                m_caseSensitivity = Qt::CaseSensitive;
                ++handledOptions;
            }
            else if (option.endsWith(QLatin1String("third-party"))) {
                setOption(ThirdPartyOption);
                setException(ThirdPartyOption, option.startsWith(QLatin1Char('~')));
                ++handledOptions;
            }
            else if (option.endsWith(QLatin1String("object"))) {
                setOption(ObjectOption);
                setException(ObjectOption, option.startsWith(QLatin1Char('~')));
                ++handledOptions;
            }
            else if (option.endsWith(QLatin1String("subdocument"))) {
                setOption(SubdocumentOption);
                setException(SubdocumentOption, option.startsWith(QLatin1Char('~')));
                ++handledOptions;
            }
            else if (option.endsWith(QLatin1String("xmlhttprequest"))) {
                setOption(XMLHttpRequestOption);
                setException(XMLHttpRequestOption, option.startsWith(QLatin1Char('~')));
                ++handledOptions;
            }
            else if (option.endsWith(QLatin1String("image"))) {
                setOption(ImageOption);
                setException(ImageOption, option.startsWith(QLatin1Char('~')));
                ++handledOptions;
            }
            else if (option == QLatin1String("document") && m_isException) {
                setOption(DocumentOption);
                ++handledOptions;
            }
            else if (option == QLatin1String("elemhide") && m_isException) {
                setOption(ElementHideOption);
                ++handledOptions;
            }
            else if (option == QLatin1String("collapse")) {
                // Hiding placeholders of blocked elements is enabled by default
                ++handledOptions;
            }
        }

        // If we don't handle all options, it's safer to just disable this rule
        if (handledOptions != options.count()) {
            m_isInternalDisabled = true;
            m_type = Invalid;
            return;
        }

        parsedLine = parsedLine.left(optionsIndex);
    }

    // Rule is classic regexp
    if (parsedLine.startsWith(QLatin1Char('/')) && parsedLine.endsWith(QLatin1Char('/'))) {
        parsedLine = parsedLine.mid(1);
        parsedLine = parsedLine.left(parsedLine.size() - 1);

        m_type = RegExpMatchRule;
        m_regExp = new RegExp;
        m_regExp->regExp = QzRegExp(parsedLine, m_caseSensitivity);
        m_regExp->regExpStrings = parseRegExpFilter(parsedLine);
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
    if (parsedLine.startsWith(QLatin1String("||")) &&
        parsedLine.endsWith(QLatin1Char('^')) &&
        !parsedLine.contains(QzRegExp("[/:?=&\\*]"))
       ) {
        parsedLine = parsedLine.mid(2);
        parsedLine = parsedLine.left(parsedLine.size() - 1);

        m_type = DomainMatchRule;
        m_matchString = parsedLine;
        return;
    }

    // If rule contains only | at end, we can also use string matching
    if (parsedLine.endsWith(QLatin1Char('|')) &&
        parsedLine.count(QLatin1Char('|')) == 1 &&
        !parsedLine.contains(QzRegExp("[\\^\\*]"))
       ) {
        parsedLine = parsedLine.left(parsedLine.size() - 1);

        m_type = StringEndsMatchRule;
        m_matchString = parsedLine;
        return;
    }

    // If we still find a wildcard (*) or separator (^) or (|)
    // we must modify parsedLine to comply with QzRegExp
    if (parsedLine.contains(QLatin1Char('*')) ||
        parsedLine.contains(QLatin1Char('^')) ||
        parsedLine.contains(QLatin1Char('|'))
       ) {
        QString parsedRegExp = parsedLine;

        parsedRegExp.replace(QzRegExp(QLatin1String("\\*+")), QLatin1String("*"))       // remove multiple wildcards
        .replace(QzRegExp(QLatin1String("\\^\\|$")), QLatin1String("^"))    // remove anchors following separator placeholder
        .replace(QzRegExp(QLatin1String("^(\\*)")), QString())      // remove leading wildcards
        .replace(QzRegExp(QLatin1String("(\\*)$")), QString())
        .replace(QzRegExp(QLatin1String("(\\W)")), QLatin1String("\\\\1"))  // escape special symbols
        .replace(QzRegExp(QLatin1String("^\\\\\\|\\\\\\|")),
                 QLatin1String("^[\\w\\-]+:\\/+(?!\\/)(?:[^\\/]+\\.)?"))   // process extended anchor at expression start
        .replace(QzRegExp(QLatin1String("\\\\\\^")),
                 QLatin1String("(?:[^\\w\\d\\-.%]|$)"))                    // process separator placeholders
        .replace(QzRegExp(QLatin1String("^\\\\\\|")), QLatin1String("^"))   // process anchor at expression start
        .replace(QzRegExp(QLatin1String("\\\\\\|$")), QLatin1String("$"))   // process anchor at expression end
        .replace(QzRegExp(QLatin1String("\\\\\\*")), QLatin1String(".*"));  // replace wildcards by .*

        m_type = RegExpMatchRule;
        m_regExp = new RegExp;
        m_regExp->regExp = QzRegExp(parsedRegExp, m_caseSensitivity);
        m_regExp->regExpStrings = parseRegExpFilter(parsedLine);
        return;
    }

    // We haven't found anything that needs use of regexp, yay!
    m_type = StringContainsMatchRule;
    m_matchString = parsedLine;
}

void AdBlockRule::parseDomains(const QString &domains, const QChar &separator)
{
    QStringList domainsList = domains.split(separator, QString::SkipEmptyParts);

    foreach (const QString domain, domainsList) {
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

    if (!m_blockedDomains.isEmpty() || !m_allowedDomains.isEmpty()) {
        setOption(DomainRestrictedOption);
    }
}

bool AdBlockRule::isMatchingDomain(const QString &domain, const QString &filter) const
{
    return QzTools::matchDomain(filter, domain);
}

bool AdBlockRule::isMatchingRegExpStrings(const QString &url) const
{
    Q_ASSERT(m_regExp);

    foreach (const QString &string, m_regExp->regExpStrings) {
        if (!url.contains(string)) {
            return false;
        }
    }

    return true;
}

// Split regexp filter into strings that can be used with QString::contains
// Don't use parts that contains only 1 char and duplicated parts
QStringList AdBlockRule::parseRegExpFilter(const QString &parsedFilter) const
{
    // Meta characters in AdBlock rules are | * ^
    QStringList list = parsedFilter.split(QzRegExp("[|\\*\\^]"), QString::SkipEmptyParts);

    list.removeDuplicates();

    for (int i = 0; i < list.length(); ++i) {
        const QString part = list.at(i);

        if (part.length() < 2) {
            list.removeAt(i);
            i--;
        }
    }

    return list;
}

bool AdBlockRule::hasOption(const AdBlockRule::RuleOption &opt) const
{
    return (m_options & opt);
}

bool AdBlockRule::hasException(const AdBlockRule::RuleOption &opt) const
{
    return (m_exceptions & opt);
}

void AdBlockRule::setOption(const AdBlockRule::RuleOption &opt)
{
    m_options |= opt;
}

void AdBlockRule::setException(const AdBlockRule::RuleOption &opt, bool on)
{
    if (on) {
        m_exceptions |= opt;
    }
}

AdBlockRule::~AdBlockRule()
{
    delete m_regExp;
}
