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
#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>

#include "qz_namespace.h"

class QMouseEvent;

class QT_QUPZILLA_EXPORT ClickableLabel : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(QSize fixedsize READ size WRITE setFixedSize)
    Q_PROPERTY(int fixedwidth READ width WRITE setFixedWidth)
    Q_PROPERTY(int fixedheight READ height WRITE setFixedHeight)

public:
    explicit ClickableLabel(QWidget* parent = 0);

signals:
    void clicked(QPoint);
    void middleClicked(QPoint);

private:
    void mouseReleaseEvent(QMouseEvent* ev);

};

#endif // CLICKABLELABEL_H
