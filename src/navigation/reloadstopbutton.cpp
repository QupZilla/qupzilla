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
#include "reloadstopbutton.h"

ReloadStopButton::ReloadStopButton(QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* lay = new QHBoxLayout(this);
    setLayout(lay);

    m_buttonStop = new ToolButton(this);
    m_buttonStop->setObjectName("navigation-button-stop");
    m_buttonStop->setToolTip(tr("Stop"));
    m_buttonStop->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_buttonStop->setVisible(false);
    m_buttonStop->setAutoRaise(true);
    m_buttonStop->setFocusPolicy(Qt::NoFocus);

    m_buttonReload = new ToolButton(this);
    m_buttonReload->setObjectName("navigation-button-reload");
    m_buttonReload->setToolTip(tr("Reload"));
    m_buttonReload->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_buttonReload->setAutoRaise(true);
    m_buttonReload->setFocusPolicy(Qt::NoFocus);

    lay->addWidget(m_buttonStop);
    lay->addWidget(m_buttonReload);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);
}

void ReloadStopButton::showReloadButton()
{
    setUpdatesEnabled(false);
    m_buttonStop->hide();
    m_buttonReload->show();
    setUpdatesEnabled(true);
}

void ReloadStopButton::showStopButton()
{
    setUpdatesEnabled(false);
    m_buttonReload->hide();
    m_buttonStop->show();
    setUpdatesEnabled(true);
}

ReloadStopButton::~ReloadStopButton()
{
}
