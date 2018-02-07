/* ============================================================
* VerticalTabs plugin for QupZilla
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
#include "verticaltabsschemehandler.h"

#include "qztools.h"

#include <QIcon>
#include <QUrlQuery>
#include <QWebEngineUrlRequestJob>

VerticalTabsSchemeHandler::VerticalTabsSchemeHandler(QObject *parent)
    : ExtensionSchemeHandler(parent)
{
}

void VerticalTabsSchemeHandler::requestStarted(QWebEngineUrlRequestJob *job)
{
    const auto parts = job->requestUrl().path().split(QL1C('/'), QString::SkipEmptyParts);
    if (!parts.isEmpty()) {
        if (parts.at(0) == QL1S("group")) {
            setReply(job, QByteArrayLiteral("text/html"), groupPage());
            return;
        }
    }
    setReply(job, QByteArrayLiteral("text/html"), indexPage());
}

QByteArray VerticalTabsSchemeHandler::indexPage() const
{
    QString page = QzTools::readAllFileContents(QSL(":verticaltabs/data/index.html"));

    page.replace(QSL("%NAME%"), tr("Vertical Tabs"));

    return page.toUtf8();
}

QByteArray VerticalTabsSchemeHandler::groupPage() const
{
    QString page = QzTools::readAllFileContents(QSL(":verticaltabs/data/group.html"));

    page.replace(QSL("%FAVICON%"), QzTools::pixmapToDataUrl(QIcon(QSL(":verticaltabs/data/group.svg")).pixmap(16)).toString());
    page.replace(QSL("%NEW-GROUP%"), tr("New Group"));

    return page.toUtf8();
}
