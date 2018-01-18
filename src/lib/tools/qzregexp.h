/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2013-2014  David Rosca <nowrep@gmail.com>
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
#ifndef QZREGEXP_H
#define QZREGEXP_H

#include <QRegularExpression>
#include <QStringList>

#include "qzcommon.h"

class QUPZILLA_EXPORT QzRegExp : public QRegularExpression
{
public:
    QzRegExp();
    QzRegExp(const QString &pattern, Qt::CaseSensitivity cs = Qt::CaseSensitive);
    QzRegExp(const QzRegExp &re);

    void setMinimal(bool minimal);
    int indexIn(const QString &str, int offset = 0) const;
    int matchedLength() const;
    QString cap(int nth = 0) const;

private:
    QStringList m_capturedTexts;
    int m_matchedLength;

};

#endif // QZREGEXP_H
