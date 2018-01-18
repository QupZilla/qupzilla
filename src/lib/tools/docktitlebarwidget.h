/* ============================================================
* QupZilla - WebKit based browser
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
#ifndef DOCKTITLEBARWIDGET_H
#define DOCKTITLEBARWIDGET_H

#include <QWidget>

#include "qzcommon.h"
#include "ui_docktitlebarwidget.h"

class QUPZILLA_EXPORT DockTitleBarWidget : public QWidget, public Ui_DockTitleBarWidget
{
public:
    explicit DockTitleBarWidget(const QString &title, QWidget* parent = 0);
    ~DockTitleBarWidget();

    void setTitle(const QString &title);

private:
};

#endif // DOCKTITLEBARWIDGET_H
