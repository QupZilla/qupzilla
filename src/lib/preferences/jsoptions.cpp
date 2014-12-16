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
#include "qztools.h"
#include "settings.h"

#include <QWebPage> // QTWEBKIT_VERSION_CHECK macro
#include <QFileDialog>
#include <QMessageBox>
#include <QShortcut>
#include <QTimer>
#include <QInputDialog>
#include <QCloseEvent>

JsOptions::JsOptions(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::JsOptions)
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);

    if (!parent)
        QzTools::centerWidgetOnScreen(this);

    if (isRightToLeft()) {
        ui->jswhiteList->setLayoutDirection(Qt::LeftToRight);
        ui->jsblackList->setLayoutDirection(Qt::LeftToRight);
    }
    // Site Filtering
    connect(ui->jsWhiteAdd, SIGNAL(clicked()), this, SLOT(jsAddWhitelist()));
    connect(ui->jsWhiteRemove, SIGNAL(clicked()), this, SLOT(jsRemoveWhitelist()));
    connect(ui->jsBlackAdd, SIGNAL(clicked()), this, SLOT(jsAddBlacklist()));
    connect(ui->jsBlackRemove, SIGNAL(clicked()), this, SLOT(jsRemoveBlacklist()));
    connect(ui->close, SIGNAL(clicked(QAbstractButton*)), this, SLOT(close()));
    connect(ui->close2, SIGNAL(clicked(QAbstractButton*)), this, SLOT(close()));

    Settings settings;
    settings.beginGroup("JavaScript-Settings");
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

    QShortcut* removeShortcut = new QShortcut(QKeySequence("Del"), this);
    connect(removeShortcut, SIGNAL(activated()), this, SLOT(deletePressed()));

    QzTools::setWmClass("JavaScript", this);

    QTimer::singleShot(0, this, SLOT(slotRefreshFilters()));
}

void JsOptions::slotRefreshFilters()
{
    ui->jswhiteList->clear();
    ui->jsblackList->clear();

    Settings settings;
    settings.beginGroup("JavaScript-Settings");
    QStringList jswhiteList = settings.value("jswhitelist", QStringList()).toStringList();
    QStringList jsblackList = settings.value("jsblacklist", QStringList()).toStringList();
    settings.endGroup();

    ui->jswhiteList->addItems(jswhiteList);
    ui->jsblackList->addItems(jsblackList);
}

void JsOptions::jsAddWhitelist()
{
    const QString server = QInputDialog::getText(this, tr("Add to whitelist"), tr("Server:"));

    if (server.isEmpty()) {
        return;
    }

    if (!ui->jsblackList->findItems(server, Qt::MatchFixedString).isEmpty()) {
        QMessageBox::information(this, tr("Already blacklisted!"), tr("The server \"%1\" is already in blacklist, please remove it first.").arg(server));
        return;
    }

    if (ui->jswhiteList->findItems(server, Qt::MatchFixedString).isEmpty()) {
        ui->jswhiteList->addItem(server);
    }
}

void JsOptions::jsRemoveWhitelist()
{
    delete ui->jswhiteList->currentItem();
}

void JsOptions::jsAddBlacklist()
{
    const QString server = QInputDialog::getText(this, tr("Add to blacklist"), tr("Server:"));

    if (server.isEmpty()) {
        return;
    }

    if (!ui->jswhiteList->findItems(server, Qt::MatchFixedString).isEmpty()) {
        QMessageBox::information(this, tr("Already whitelisted!"), tr("The server \"%1\" is already in whitelist, please remove it first.").arg(server));
        return;
    }

    if (ui->jsblackList->findItems(server, Qt::MatchFixedString).isEmpty()) {
        ui->jsblackList->addItem(server);
    }
}

void JsOptions::jsRemoveBlacklist()
{
    delete ui->jsblackList->currentItem();
}

void JsOptions::deletePressed()
{
    if (ui->jswhiteList->hasFocus()) {
        jsRemoveWhitelist();
    }
    else if (ui->jsblackList->hasFocus()) {
        jsRemoveBlacklist();
    }
}

void JsOptions::closeEvent(QCloseEvent* e)
{
    QStringList jswhitelist;
    QStringList jsblacklist;

    for (int i = 0; i < ui->jswhiteList->count(); ++i) {
        jswhitelist.append(ui->jswhiteList->item(i)->text());
    }

    for (int i = 0; i < ui->jsblackList->count(); ++i) {
        jsblacklist.append(ui->jsblackList->item(i)->text());
    }

    Settings settings;
    settings.beginGroup("JavaScript-Settings");
    settings.setValue("allowJavaScriptCloseWindow", ui->jscanCloseWindow->isChecked());
    settings.setValue("allowJavaScriptOpenWindow", ui->jscanOpenWindow->isChecked());
    settings.setValue("allowJavaScriptGeometryChange", ui->jscanChangeSize->isChecked());
    settings.setValue("allowJavaScriptHideMenuBar", ui->jscanHideMenu->isChecked());
    settings.setValue("allowJavaScriptHideStatusBar", ui->jscanHideStatus->isChecked());
    settings.setValue("allowJavaScriptHideToolBar", ui->jscanHideTool->isChecked());
    settings.setValue("allowJavaScriptAccessClipboard", ui->jscanAccessClipboard->isChecked());
    settings.setValue("jswhitelist", jswhitelist);
    settings.setValue("jsblacklist", jsblacklist);
    settings.endGroup();

    e->accept();
}

void JsOptions::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape) {
        close();
    }

    QWidget::keyPressEvent(e);
}

JsOptions::~JsOptions()
{
    delete ui;
}
