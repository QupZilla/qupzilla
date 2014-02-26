/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
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
#ifndef CHECKBOXDIALOG_H
#define CHECKBOXDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>

#include "qzcommon.h"

namespace Ui
{
class CheckBoxDialog;
}

class QUPZILLA_EXPORT CheckBoxDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CheckBoxDialog(const QDialogButtonBox::StandardButtons &buttons, QWidget* parent = 0);

    void setIcon(const QIcon &icon);

    void setText(const QString &text);
    void setCheckBoxText(const QString &text);

    bool isChecked() const;
    void setDefaultCheckState(Qt::CheckState state);

public slots:
    int exec();

private:
    Ui::CheckBoxDialog* ui;

};

#endif // CHECKBOXDIALOG_H
