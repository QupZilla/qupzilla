/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2015 David Rosca <nowrep@gmail.com>
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

#include "scripts.h"

QString Scripts::setCss(const QString &css)
{
    static QString source = QL1S("(function() {"
                                 "var css = document.createElement('style');"
                                 "css.setAttribute('type', 'text/css');"
                                 "css.appendChild(document.createTextNode('%1'));"
                                 "document.getElementsByTagName('head')[0].appendChild(css);"
                                 "})()");

    QString style = css;
    style.replace(QL1S("'"), QL1S("\\'"));
    style.replace(QL1S("\n"), QL1S("\\n"));
    return source.arg(style);
}
