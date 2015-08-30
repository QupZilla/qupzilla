/* ============================================================
* AutoScroll - Autoscroll for QupZilla
* Copyright (C) 2014  David Rosca <nowrep@gmail.com>
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
#ifndef AUTOSCROLLSETTINGS_H
#define AUTOSCROLLSETTINGS_H

#include <QDialog>

namespace Ui
{
class AutoScrollSettings;
}

class AutoScroller;

class AutoScrollSettings : public QDialog
{
    Q_OBJECT

public:
    explicit AutoScrollSettings(const QString &settingsPath, QWidget *parent = Q_NULLPTR);
    ~AutoScrollSettings();

signals:
    void settingsChanged();

private slots:
    void accepted();

private:
    Ui::AutoScrollSettings* ui;
    QString m_settingsPath;
};

#endif // AUTOSCROLLSETTINGS_H
