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
#include "enhancedmenu.h"

#include <QMouseEvent>
#include <QApplication>

Menu::Menu(QWidget* parent)
    : QMenu(parent)
{
}

Menu::Menu(const QString &title, QWidget* parent)
    : QMenu(title, parent)
{
}

void Menu::mouseReleaseEvent(QMouseEvent* e)
{
    QAction* qact = actionAt(e->pos());
    Action* act = qobject_cast<Action*> (qact);

    if (qact && qact->menu()) {
        Menu* m = qobject_cast<Menu*> (qact->menu());
        if (!m) {
            QMenu::mouseReleaseEvent(e);
            return;
        }

        if (e->button() == Qt::MiddleButton || (e->button() == Qt::LeftButton && e->modifiers() == Qt::ControlModifier)) {
            closeAllMenus();
            emit menuMiddleClicked(m);
        }
    }

    if (!act) {
        QMenu::mouseReleaseEvent(e);
        return;
    }

    if ((e->button() == Qt::LeftButton || e->button() == Qt::RightButton) && e->modifiers() == Qt::NoModifier) {
        closeAllMenus();
        act->trigger();
        e->accept();
    }
    else if (e->button() == Qt::MiddleButton || (e->button() == Qt::LeftButton && e->modifiers() == Qt::ControlModifier)) {
        closeAllMenus();
        act->triggerMiddleClick();
        e->accept();
    }
}

void Menu::closeAllMenus()
{
    QMenu* menu = this;

    while (menu) {
        menu->close();
        menu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
    }
}

Action::Action(QObject* parent)
    : QAction(parent)
{
}

Action::Action(const QString &text, QObject* parent)
    : QAction(text, parent)
{
}

Action::Action(const QIcon &icon, const QString &text, QObject* parent)
    : QAction(icon, text, parent)
{
}

void Action::triggerMiddleClick()
{
    emit middleClicked();
}
