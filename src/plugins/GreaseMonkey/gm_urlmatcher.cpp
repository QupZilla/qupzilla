/* ============================================================
* GreaseMonkey plugin for QupZilla
* Copyright (C) 2012  David Rosca <nowrep@gmail.com>
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
#include "gm_urlmatcher.h"

#include <QDebug>
#include <QStringList>

bool wildcardMatch(const QString &string, const QString &pattern)
{
    int stringSize = string.size();
    int patternSize = pattern.size();

    bool startsWithWildcard = pattern[0] == '*';
    bool endsWithWildcard = pattern[patternSize - 1] == '*';

    const QStringList &parts = pattern.split('*');
    int pos = 0;

    if (startsWithWildcard) {
        pos = string.indexOf(parts.at(1));
        if (pos == -1) {
            return false;
        }
    }

    foreach(const QString & part, parts) {
        pos = string.indexOf(part, pos);
        if (pos == -1) {
            return false;
        }
    }

    if (!endsWithWildcard && stringSize - pos != parts.last().size()) {
        return false;
    }

    return true;
}

GM_UrlMatcher::GM_UrlMatcher(const QString &pattern)
    : m_pattern(pattern)
    , m_useRegExp(false)
{
    parsePattern(m_pattern);
}

QString GM_UrlMatcher::pattern() const
{
    return m_pattern;
}

bool GM_UrlMatcher::match(const QString &urlString) const
{
    if (m_useRegExp) {
        return m_regExp.indexIn(urlString) != -1;
    }
    else {
        return wildcardMatch(urlString, m_matchString);
    }
}

void GM_UrlMatcher::parsePattern(QString pattern)
{
    if (pattern.startsWith('/') && pattern.endsWith('/')) {
        pattern = pattern.mid(1);
        pattern = pattern.left(pattern.size() - 1);

        m_regExp = QRegExp(pattern, Qt::CaseInsensitive);
        m_useRegExp = true;
        return;
    }

    if (pattern.contains(".tld")) {

        pattern.replace(QRegExp("(\\W)"), "\\\\1")
        .replace(QRegExp("\\*+"), "*")
        .replace(QRegExp("^\\\\\\|"), "^")
        .replace(QRegExp("\\\\\\|$"), "$")
        .replace(QRegExp("\\\\\\*"), ".*")
        .replace("\\.tld", "\\.[a-z.]{2,6}");

        m_useRegExp = true;
        m_regExp = QRegExp(pattern, Qt::CaseInsensitive);
    }
    else {
        m_matchString = pattern;
    }
}
