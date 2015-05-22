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
#include "sbi_icon.h"
#include "browserwindow.h"
#include "tabbedwebview.h"
#include "webpage.h"

SBI_Icon::SBI_Icon(BrowserWindow* window, const QString &settingsPath)
    : ClickableLabel(window)
    , m_window(window)
    , m_settingsFile(settingsPath + QL1S("/extensions.ini"))
{
}

bool SBI_Icon::testCurrentPageWebAttribute(QWebEngineSettings::WebAttribute attr) const
{
    return currentPageSettings() && currentPageSettings()->testAttribute(attr);
}

void SBI_Icon::setCurrentPageWebAttribute(QWebEngineSettings::WebAttribute attr, bool value)
{
    if (currentPageSettings()) {
        currentPageSettings()->setAttribute(attr, value);
    }
}

QWebEngineSettings* SBI_Icon::currentPageSettings() const
{
    if (!m_window->weView()) {
        return 0;
    }

    return m_window->weView()->page()->settings();
}

WebPage* SBI_Icon::currentPage() const
{
    if (!m_window->weView()) {
        return 0;
    }

    return m_window->weView()->page();
}

