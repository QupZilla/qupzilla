/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2018  David Rosca <nowrep@gmail.com>
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
#include "qztools.h"

#include <QFont>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QDialogButtonBox>

LicenseViewer::LicenseViewer(QWidget* parent)
    : QWidget()
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("License Viewer"));

    m_textBrowser = new QTextBrowser(this);

    QFont serifFont = m_textBrowser->font();
    serifFont.setFamily("Courier");
    m_textBrowser->setFont(serifFont);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
    buttonBox->setStandardButtons(QDialogButtonBox::Close);
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));

    QVBoxLayout* l = new QVBoxLayout(this);
    l->addWidget(m_textBrowser);
    l->addWidget(buttonBox);

    setLayout(l);

    resize(600, 500);

    QzTools::centerWidgetToParent(this, parent);
}

void LicenseViewer::setLicenseFile(const QString &fileName)
{
    m_textBrowser->setText(QzTools::readAllFileContents(fileName));
}

void LicenseViewer::setText(const QString &text)
{
    m_textBrowser->setText(text);
}
