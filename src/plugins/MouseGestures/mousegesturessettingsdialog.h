/* ============================================================
* Mouse Gestures plugin for QupZilla
* Copyright (C) 2012-2014  David Rosca <nowrep@gmail.com>
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
#ifndef MOUSEGESTURESSETTINGSDIALOG_H
#define MOUSEGESTURESSETTINGSDIALOG_H

#include <QDialog>
#include <QTextBrowser>

namespace Ui
{
class MouseGesturesSettingsDialog;
}

class MouseGestures;

class MouseGesturesSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MouseGesturesSettingsDialog(MouseGestures* gestures, QWidget* parent = 0);
    ~MouseGesturesSettingsDialog();

private slots:
    void showLicense();
    void accepted();

private:
    Ui::MouseGesturesSettingsDialog* ui;
    MouseGestures* m_gestures;
};

#endif // MOUSEGESTURESSETTINGSDIALOG_H
