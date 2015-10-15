/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2013-2014  David Rosca <nowrep@gmail.com>
                2013-2014  Mladen Pejaković <pejakm@autistici.org>
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
#include "jsoptions.h"
#include "ui_jsoptions.h"
#include "mainapplication.h"
#include "settings.h"

JsOptions::JsOptions(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::JsOptions)
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);

    Settings settings;
    settings.beginGroup("Web-Browser-Settings");
    ui->jscanOpenWindow->setChecked(settings.value("allowJavaScriptOpenWindow", false).toBool());
    ui->jscanChangeSize->setChecked(settings.value("allowJavaScriptGeometryChange", true).toBool());
    ui->jscanAccessClipboard->setChecked(settings.value("allowJavaScriptAccessClipboard", false).toBool());
    settings.endGroup();
}

void JsOptions::accept()
{
    Settings settings;
    settings.beginGroup("Web-Browser-Settings");
    settings.setValue("allowJavaScriptOpenWindow", ui->jscanOpenWindow->isChecked());
    settings.setValue("allowJavaScriptGeometryChange", ui->jscanChangeSize->isChecked());
    settings.setValue("allowJavaScriptAccessClipboard", ui->jscanAccessClipboard->isChecked());
    settings.endGroup();

    QDialog::close();
}

JsOptions::~JsOptions()
{
    delete ui;
}
