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

#include <QWebPage> // QTWEBKIT_VERSION_CHECK macro
#include <QFileDialog>

JsOptions::JsOptions(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::JsOptions)
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);

    Settings settings;
    settings.beginGroup("Web-Browser-Settings");
    ui->jscanCloseWindow->setChecked(settings.value("allowJavaScriptCloseWindow", false).toBool());
    ui->jscanOpenWindow->setChecked(settings.value("allowJavaScriptOpenWindow", false).toBool());
    ui->jscanChangeSize->setChecked(settings.value("allowJavaScriptGeometryChange", true).toBool());
    ui->jscanHideMenu->setChecked(settings.value("allowJavaScriptHideMenuBar", true).toBool());
    ui->jscanHideStatus->setChecked(settings.value("allowJavaScriptHideStatusBar", true).toBool());
    ui->jscanHideTool->setChecked(settings.value("allowJavaScriptHideToolBar", true).toBool());
    ui->jscanAccessClipboard->setChecked(settings.value("allowJavaScriptAccessClipboard", false).toBool());
    settings.endGroup();

#if QTWEBKIT_TO_2_2
    ui->jscanCloseWindow->setHidden(true);
#endif
//     Disable for now, as it does not do anything (yet)
    ui->jscanHideTool->setHidden(true);
}

void JsOptions::accept()
{
    Settings settings;
    settings.beginGroup("Web-Browser-Settings");
    settings.setValue("allowJavaScriptCloseWindow", ui->jscanCloseWindow->isChecked());
    settings.setValue("allowJavaScriptOpenWindow", ui->jscanOpenWindow->isChecked());
    settings.setValue("allowJavaScriptGeometryChange", ui->jscanChangeSize->isChecked());
    settings.setValue("allowJavaScriptHideMenuBar", ui->jscanHideMenu->isChecked());
    settings.setValue("allowJavaScriptHideStatusBar", ui->jscanHideStatus->isChecked());
    settings.setValue("allowJavaScriptHideToolBar", ui->jscanHideTool->isChecked());
    settings.setValue("allowJavaScriptAccessClipboard", ui->jscanAccessClipboard->isChecked());
    settings.endGroup();

    QDialog::close();
}

JsOptions::~JsOptions()
{
    delete ui;
}
