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

// Version for Qt < 4.8 has one issue, it will wrongly
// count .co.uk (and others) as second-level domain
QString toSecondLevelDomain(const QUrl &url)
{
#if QT_VERSION >= 0x040800
    const QString &topLevelDomain = url.topLevelDomain();
    const QString &urlHost = url.host();

    if (topLevelDomain.isEmpty() || urlHost.isEmpty()) {
        return QString();
    }

    QString domain = urlHost.left(urlHost.size() - topLevelDomain.size());

    if (domain.count('.') == 0) {
        return urlHost;
    }

    while (domain.count('.') != 0) {
        domain = domain.mid(domain.indexOf('.') + 1);
    }

    return domain + topLevelDomain;
#else
    QString domain = url.host();

    if (domain.count('.') == 0) {
        return QString();
    }

    while (domain.count('.') != 1) {
        domain = domain.mid(domain.indexOf('.') + 1);
    }

    return domain;
#endif
}

AdBlockRule::AdBlockRule(const QString &filter)
    : m_enabled(true)
    , m_cssRule(false)
    , m_exception(false)
    , m_internalDisabled(false)
    , m_domainRestricted(false)
    , m_useRegExp(false)
    , m_thirdParty(false)
    , m_thirdPartyException(false)
    , m_caseSensitivity(Qt::CaseInsensitive)
{
    setFilter(filter);
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

bool AdBlockRule::isDomainRestricted() const
{
    return m_domainRestricted;
}

bool AdBlockRule::isException() const
{
    return m_exception;
}

bool AdBlockRule::isEnabled() const
{
    return m_enabled;
}

void AdBlockRule::setEnabled(bool enabled)
{
    m_enabled = enabled;
    if (!enabled) {
        m_filter = QLatin1String("!") + m_filter;
    }
    else {
        m_filter = m_filter.mid(1);
    }
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

    if (m_useRegExp) {
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
    }

    return matched;
}

bool AdBlockRule::matchDomain(const QString &domain) const
{
    if (!m_domainRestricted) {
        return true;
    }

    if (m_blockedDomains.isEmpty()) {
        foreach(const QString & d, m_allowedDomains) {
            if (domain.contains(d)) {
                return true;
            }
        }
    }
    else if (m_allowedDomains.isEmpty()) {
        foreach(const QString & d, m_blockedDomains) {
            if (domain.contains(d)) {
                return false;
            }
        }
        return true;
    }
    else {
        foreach(const QString & d, m_blockedDomains) {
            if (domain.contains(d)) {
                return false;
            }
        }

        foreach(const QString & d, m_allowedDomains) {
            if (domain.contains(d)) {
                return true;
            }
        }
    }

    return false;
}

bool AdBlockRule::matchThirdParty(const QNetworkRequest &request) const
{
    const QString &referer = request.rawHeader("Referer");
    if (referer.isEmpty()) {
        return false;
    }

    // Third-party matching should be performed on second-level domains
    const QString &refererHost = toSecondLevelDomain(QUrl(referer));
    const QString &host = toSecondLevelDomain(request.url());

    return m_thirdPartyException ? refererHost == host : refererHost != host;
}

void AdBlockRule::parseFilter()
{
    QString parsedLine = m_filter;

    // Empty rule
    if (m_filter.trimmed().isEmpty()) {
        m_enabled = false;
        return;
    }

    // Disabled rule - modify parsedLine to not contain starting ! so we can continue parsing rule
    if (m_filter.startsWith('!')) {
        m_enabled = false;
        parsedLine = m_filter.mid(1);
    }

    // CSS Element hiding rule
    if (parsedLine.contains("##")) {
        m_cssRule = true;
        int pos = parsedLine.indexOf("##");

        // Domain restricted rule
        if (!parsedLine.startsWith("##")) {
            QString domains = parsedLine.mid(0, pos);
            parseDomains(domains, ',');
        }

        m_cssSelector = parsedLine.mid(pos + 2);
        // CSS rule cannot have more options -> stop parsing
        return;
    }

    // Exception always starts with @@
    if (parsedLine.startsWith("@@")) {
        m_exception = true;
        parsedLine = parsedLine.mid(2);
    }

    // Parse all options following $ char
    int optionsIndex = parsedLine.indexOf('$');
    if (optionsIndex >= 0) {
        QStringList options = parsedLine.mid(optionsIndex + 1).split(',');

        int handledOptions = 0;
        foreach(const QString & option, options) {
            if (option.startsWith("domain=")) {
                parseDomains(option.mid(7), '|');
                ++handledOptions;
            }
            else if (option.startsWith("match-case")) {
                m_caseSensitivity = Qt::CaseSensitive;
                ++handledOptions;
            }
            else if (option.contains("third-party")) {
                m_thirdParty = true;
                m_thirdPartyException = option.startsWith('~');
                ++handledOptions;
            }
        }

        // If we don't handle all known options, it's safer to just disable this rule
        if (handledOptions != options.count()) {
            m_internalDisabled = true;
            return;
        }

        parsedLine = parsedLine.left(optionsIndex);
    }

    // Rule is classic regexp
    if (parsedLine.startsWith('/') && parsedLine.endsWith('/')) {
        parsedLine = parsedLine.mid(1);
        parsedLine = parsedLine.left(parsedLine.size() - 1);

        m_useRegExp = true;
        m_regExp = QRegExp(parsedLine, m_caseSensitivity, QRegExp::RegExp);
        return;
    }

    // Remove starting and ending wildcards (*)
    if (parsedLine.startsWith("*")) {
        parsedLine = parsedLine.mid(1);
    }

    if (parsedLine.endsWith("*")) {
        parsedLine = parsedLine.left(parsedLine.size() - 1);
    }

    // If we still find a wildcard (*) or separator (^) or (|)
    // we must modify parsedLine to comply with QRegExp
    if (parsedLine.contains('*') || parsedLine.contains('^') || parsedLine.contains('|')) {
        parsedLine.replace(QRegExp(QLatin1String("\\*+")), QLatin1String("*"))       // remove multiple wildcards
        .replace(QRegExp(QLatin1String("\\^\\|$")), QLatin1String("^"))    // remove anchors following separator placeholder
        .replace(QRegExp(QLatin1String("^(\\*)")), QLatin1String(""))      // remove leading wildcards
        .replace(QRegExp(QLatin1String("(\\*)$")), QLatin1String(""))
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
        if (domain.startsWith('~')) {
            m_blockedDomains.append(domain.mid(1));
        }
        else {
            m_allowedDomains.append(domain);
        }
    }

    m_domainRestricted = (!m_blockedDomains.isEmpty() || !m_allowedDomains.isEmpty());
}
