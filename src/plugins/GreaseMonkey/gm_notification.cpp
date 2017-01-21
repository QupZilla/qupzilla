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
#include "gm_notification.h"
#include "ui_gm_notification.h"
#include "gm_manager.h"
#include "gm_script.h"

#include "iconprovider.h"

#include <QFile>

GM_Notification::GM_Notification(GM_Manager* manager, const QString &tmpfileName, const QString &fileName)
    : AnimatedWidget(AnimatedWidget::Down, 300, 0)
    , ui(new Ui::GM_Notification)
    , m_manager(manager)
    , m_tmpFileName(tmpfileName)
    , m_fileName(fileName)
{
    setAutoFillBackground(true);
    ui->setupUi(widget());

    ui->iconLabel->setPixmap(QIcon(QSL(":gm/data/icon.svg")).pixmap(24));
    ui->close->setIcon(IconProvider::standardIcon(QStyle::SP_DialogCloseButton));

    connect(ui->install, SIGNAL(clicked()), this, SLOT(installScript()));
    connect(ui->close, SIGNAL(clicked()), this, SLOT(hide()));

    startAnimation();
}

void GM_Notification::installScript()
{
    bool success = false;

    GM_Script* script = 0;
    QString message = tr("Cannot install script");

    if (QFile::copy(m_tmpFileName, m_fileName)) {
        script = new GM_Script(m_manager, m_fileName);
        success = m_manager->addScript(script);
    }

    if (success) {
        message = tr("'%1' installed successfully").arg(script->name());
    }

    m_manager->showNotification(message);

    hide();
}

GM_Notification::~GM_Notification()
{
    delete ui;
}
