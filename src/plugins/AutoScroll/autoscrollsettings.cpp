/* ============================================================
* AutoScroll - Autoscroll for QupZilla
* Copyright (C) 2014-2017 David Rosca <nowrep@gmail.com>
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
#include "autoscrollsettings.h"
#include "ui_autoscrollsettings.h"
#include "autoscroller.h"

#include <QIcon>

AutoScrollSettings::AutoScrollSettings(AutoScroller* scroller, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::AutoScrollSettings)
    , m_scroller(scroller)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);
    ui->divider->setValue(m_scroller->scrollDivider());
    ui->iconLabel->setPixmap(QIcon(QStringLiteral(":/autoscroll/data/scroll_all.png")).pixmap(32));

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accepted()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(close()));
}

AutoScrollSettings::~AutoScrollSettings()
{
    delete ui;
}

void AutoScrollSettings::accepted()
{
    m_scroller->setScrollDivider(ui->divider->value());
    close();
}
