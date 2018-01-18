/* ============================================================
* QupZilla - Qt web browser
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
#include "reloadstopbutton.h"

#include <QTimer>
#include <QStyle>

ReloadStopButton::ReloadStopButton(QWidget* parent)
    : ToolButton(parent)
    , m_loadInProgress(false)
{
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    setToolbarButtonLook(true);
    setAutoRaise(true);
    setFocusPolicy(Qt::NoFocus);

    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(50);
    m_updateTimer->setSingleShot(true);
    connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(updateButton()));

    connect(this, SIGNAL(clicked()), this, SLOT(buttonClicked()));

    updateButton();
}

void ReloadStopButton::showStopButton()
{
    m_loadInProgress = true;
    m_updateTimer->start();
}

void ReloadStopButton::showReloadButton()
{
    m_loadInProgress = false;
    m_updateTimer->start();
}

void ReloadStopButton::updateButton()
{
    if (m_loadInProgress) {
        setToolTip(tr("Stop"));
        setObjectName(QSL("navigation-button-stop"));
    }
    else {
        setToolTip(tr("Reload"));
        setObjectName(QSL("navigation-button-reload"));
    }

    // Update the stylesheet for the changed object name
    style()->unpolish(this);
    style()->polish(this);
}

void ReloadStopButton::buttonClicked()
{
    if (m_loadInProgress)
        emit stopClicked();
    else
        emit reloadClicked();
}
