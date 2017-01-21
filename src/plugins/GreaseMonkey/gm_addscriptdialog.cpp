/* ============================================================
* GreaseMonkey plugin for QupZilla
* Copyright (C) 2012-2017 David Rosca <nowrep@gmail.com>
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
#include "gm_addscriptdialog.h"
#include "ui_gm_addscriptdialog.h"
#include "gm_script.h"
#include "gm_manager.h"
#include "gm_notification.h"

#include "mainapplication.h"
#include "browserwindow.h"
#include "tabbedwebview.h"
#include "tabwidget.h"
#include "datapaths.h"
#include "qztools.h"

#include <QFile>
#include <QDir>

GM_AddScriptDialog::GM_AddScriptDialog(GM_Manager* manager, GM_Script* script, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::GM_AddScriptDialog)
    , m_manager(manager)
    , m_script(script)
{
    ui->setupUi(this);
    ui->iconLabel->setPixmap(QIcon(QSL(":gm/data/icon.svg")).pixmap(32));

    QString runsAt;
    QString dontRunsAt;

    const QStringList include = script->include();
    const QStringList exclude = script->exclude();

    if (!include.isEmpty()) {
        runsAt = tr("<p>runs at<br/><i>%1</i></p>").arg(include.join("<br/>"));
    }

    if (!exclude.isEmpty()) {
        dontRunsAt = tr("<p>does not run at<br/><i>%1</i></p>").arg(exclude.join("<br/>"));
    }

    QString scriptInfo = QString("<b>%1</b> %2<br/>%3 %4 %5").arg(script->name(), script->version(), script->description(), runsAt, dontRunsAt);
    ui->textBrowser->setText(scriptInfo);

    connect(ui->showSource, SIGNAL(clicked()), this, SLOT(showSource()));
    connect(this, SIGNAL(accepted()), this, SLOT(accepted()));
}

void GM_AddScriptDialog::showSource()
{
    BrowserWindow* qz = mApp->getWindow();
    if (!qz) {
        return;
    }

    const QString tmpFileName = QzTools::ensureUniqueFilename(DataPaths::path(DataPaths::Temp) + "/tmp-userscript.js");

    if (QFile::copy(m_script->fileName(), tmpFileName)) {
        int index = qz->tabWidget()->addView(QUrl::fromLocalFile(tmpFileName), Qz::NT_SelectedTabAtTheEnd);
        TabbedWebView* view = qz->weView(index);
        view->addNotification(new GM_Notification(m_manager, tmpFileName, m_script->fileName()));
    }

    reject();
}

void GM_AddScriptDialog::accepted()
{
    QString message = tr("Cannot install script");

    if (m_manager->addScript(m_script)) {
        message = tr("'%1' installed successfully").arg(m_script->name());
    }

    m_manager->showNotification(message);
}

GM_AddScriptDialog::~GM_AddScriptDialog()
{
    delete ui;
}
