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
#ifndef GM_URLMATCHER_H
#define GM_URLMATCHER_H

#include <QString>
#include <QRegExp>

class GM_UrlMatcher
{
public:
    GM_UrlMatcher(const QString &pattern);

    QString pattern() const;

    bool match(const QString &urlString) const;

private:
    void parsePattern(QString pattern);

    QString m_pattern;

    QString m_matchString;
    QRegExp m_regExp;

    bool m_useRegExp;
};

#endif // GM_URLMATCHER_H
