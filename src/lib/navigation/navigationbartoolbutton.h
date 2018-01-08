/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2018 David Rosca <nowrep@gmail.com>
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
#pragma once

#include "qzcommon.h"
#include "toolbutton.h"

class QLabel;

class AbstractButtonInterface;

class QUPZILLA_EXPORT NavigationBarToolButton : public ToolButton
{
    Q_OBJECT

public:
    explicit NavigationBarToolButton(AbstractButtonInterface *button, QWidget *parent = nullptr);

    void updateVisibility();

signals:
    void visibilityChangeRequested();

private:
    void clicked();
    void updateIcon();
    void updateBadge();

    void mouseReleaseEvent(QMouseEvent *e) override;

    AbstractButtonInterface *m_button;
    QLabel *m_badgeLabel = nullptr;
};
