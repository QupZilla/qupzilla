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
#ifndef CLEARPRIVATEDATA_H
#define CLEARPRIVATEDATA_H

#include <QDialog>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QLabel>

namespace Ui {
    class ClearPrivateData;
}

class QupZilla;
class ClearPrivateData : public QDialog
{
    Q_OBJECT
public:
    explicit ClearPrivateData(QupZilla* mainClass, QWidget* parent = 0);

signals:

public slots:

private slots:
    void historyClicked(bool state);
    void dialogAccepted();
    void clearFlash();

private:
    QupZilla* p_QupZilla;
    Ui::ClearPrivateData* ui;

};

#endif // CLEARPRIVATEDATA_H
