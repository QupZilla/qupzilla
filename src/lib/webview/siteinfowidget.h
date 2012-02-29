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
#ifndef SITEINFOWIDGET_H
#define SITEINFOWIDGET_H

#include <QMenu>

#include "qz_namespace.h"

namespace Ui
{
class SiteInfoWidget;
}

class QupZilla;

class QT_QUPZILLA_EXPORT SiteInfoWidget : public QMenu
{
    Q_OBJECT
public:
    explicit SiteInfoWidget(QupZilla* mainClass, QWidget* parent = 0);
    ~SiteInfoWidget();

    void showAt(QWidget* _parent);

private:
    Ui::SiteInfoWidget* ui;
    QupZilla* p_QupZilla;
};

#endif // SITEINFOWIDGET_H
