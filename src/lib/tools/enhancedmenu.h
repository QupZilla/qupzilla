/* ============================================================
* QupZilla - Qt web browser
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
#ifndef ENHANCEDMENU_H
#define ENHANCEDMENU_H

#include <QMenu>

#include "qzcommon.h"


class Action;

class QUPZILLA_EXPORT Menu : public QMenu
{
    Q_OBJECT
public:
    explicit Menu(QWidget* parent = 0);
    explicit Menu(const QString &title, QWidget* parent = 0);

    // Default is false, menu will NOT be closed on middle click
    bool closeOnMiddleClick() const;
    void setCloseOnMiddleClick(bool close);

signals:
    void menuMiddleClicked(Menu*);

public slots:

private:
    void mouseReleaseEvent(QMouseEvent* e);
    void keyPressEvent(QKeyEvent* e);

    void closeAllMenus();

    bool m_closeOnMiddleClick;
};

class QUPZILLA_EXPORT Action : public QAction
{
    Q_OBJECT
public:
    explicit Action(QObject* parent = 0);
    explicit Action(const QString &text, QObject* parent = 0);
    explicit Action(const QIcon &icon, const QString &text, QObject* parent = 0);

signals:
    void ctrlTriggered();
    void shiftTriggered();

public slots:
    void emitCtrlTriggered();
    void emitShiftTriggered();

};

#endif // ENHANCEDMENU_H
