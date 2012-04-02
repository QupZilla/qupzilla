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
#include "licenseviewer.h"
#include "globalfunctions.h"

LicenseViewer::LicenseViewer(QWidget* parent)
    : QTextBrowser()
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("License Viewer"));

    resize(450, 500);

    qz_centerWidgetToParent(this, parent);
}

void LicenseViewer::setLicenseFile(const QString &fileName)
{
    setText(qz_readAllFileContents(fileName));
}
