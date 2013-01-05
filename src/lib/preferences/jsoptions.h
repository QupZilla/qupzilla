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
#ifndef JSOPTIONS_H
#define JSOPTIONS_H

#include <QDialog>

#include "qz_namespace.h"

namespace Ui
{
class JsOptions;
}

class QT_QUPZILLA_EXPORT JsOptions : public QDialog
{
    Q_OBJECT

public slots:
    void accept();

public:
    explicit JsOptions(QWidget* parent = 0);
    ~JsOptions();

private:

    Ui::JsOptions* ui;
};

#endif // JSOPTIONS_H
