/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca
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
#ifndef RELOADSTOPBUTTON_H
#define RELOADSTOPBUTTON_H

#include <QHBoxLayout>

#include "toolbutton.h"

class ReloadStopButton : public QWidget
{
    Q_OBJECT
public:
    explicit ReloadStopButton(QWidget *parent = 0);
    ~ReloadStopButton();

    void showStopButton();
    void showReloadButton();

    ToolButton* buttonStop() { return m_buttonStop; }
    ToolButton* buttonReload() { return m_buttonReload; }

signals:

public slots:

private:
    ToolButton* m_buttonStop;
    ToolButton* m_buttonReload;
};

#endif // RELOADSTOPBUTTON_H
