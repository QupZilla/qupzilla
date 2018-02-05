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
#pragma once

#include "schemehandlers/extensionschemehandler.h"

class VerticalTabsSchemeHandler : public ExtensionSchemeHandler
{
public:
    explicit VerticalTabsSchemeHandler(QObject *parent = nullptr);

    void requestStarted(QWebEngineUrlRequestJob *job) override;

private:
    QByteArray indexPage() const;
    QByteArray groupPage() const;
};
