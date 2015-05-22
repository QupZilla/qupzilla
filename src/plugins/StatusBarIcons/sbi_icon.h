/* ============================================================
* StatusBarIcons - Extra icons in statusbar for QupZilla
* Copyright (C) 2014  David Rosca <nowrep@gmail.com>
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
#ifndef SBI_ICON_H
#define SBI_ICON_H

#include <QWebEngineSettings>

#include "clickablelabel.h"

class BrowserWindow;
class WebPage;

class SBI_Icon : public ClickableLabel
{
    Q_OBJECT

public:
    explicit SBI_Icon(BrowserWindow* window, const QString &settingsPath = QString());

protected:
    bool testCurrentPageWebAttribute(QWebEngineSettings::WebAttribute attr) const;
    void setCurrentPageWebAttribute(QWebEngineSettings::WebAttribute attr, bool value);

    QWebEngineSettings* currentPageSettings() const;
    WebPage* currentPage() const;

    BrowserWindow* m_window;
    QString m_settingsFile;
};

#endif // SBI_ICON_H
