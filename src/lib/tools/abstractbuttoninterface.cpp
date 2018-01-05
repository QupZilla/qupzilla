/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2018 David Rosca <nowrep@gmail.com>
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
#include "abstractbuttoninterface.h"

AbstractButtonInterface::AbstractButtonInterface(QObject *parent)
    : QObject(parent)
{
}

bool AbstractButtonInterface::isValid() const
{
    return !id().isEmpty() && !name().isEmpty();
}

bool AbstractButtonInterface::isActive() const
{
    return m_active;
}

void AbstractButtonInterface::setActive(bool active)
{
    if (m_active == active) {
        return;
    }

    m_active = active;
    emit activeChanged(m_active);
}

QString AbstractButtonInterface::title() const
{
    return m_title;
}

void AbstractButtonInterface::setTitle(const QString &title)
{
    if (m_title == title) {
        return;
    }

    m_title = title;
    emit titleChanged(m_title);
}

QString AbstractButtonInterface::toolTip() const
{
    return m_toolTip;
}

void AbstractButtonInterface::setToolTip(const QString &toolTip)
{
    if (m_toolTip == toolTip) {
        return;
    }

    m_toolTip = toolTip;
    emit toolTipChanged(m_toolTip);
}

QIcon AbstractButtonInterface::icon() const
{
    return m_icon;
}

void AbstractButtonInterface::setIcon(const QIcon &icon)
{
    m_icon = icon;
    emit iconChanged(icon);
}

QString AbstractButtonInterface::badgeLabelText() const
{
    return m_badgeLabelText;
}

void AbstractButtonInterface::setBadgeLabelText(const QString &badgeLabelText)
{
    if (m_badgeLabelText == badgeLabelText) {
        return;
    }

    m_badgeLabelText = badgeLabelText;
    emit badgeLabelTextChanged(m_badgeLabelText);
}

WebPage *AbstractButtonInterface::webPage() const
{
    return m_page;
}

void AbstractButtonInterface::setWebPage(WebPage *page)
{
    if (m_page == page) {
        return;
    }

    m_page = page;
    emit webPageChanged(m_page);
}
