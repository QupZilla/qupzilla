#include "menu.h"

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
    Action* act = qobject_cast<Action*> (actionAt(e->pos()));

    if (!act) {
        QMenu::mouseReleaseEvent(e);
        return;
    }

    if (e->button() == Qt::LeftButton && e->modifiers() == Qt::NoModifier) {
        act->trigger();
        closeAllMenus();
        e->accept();
    }
    else if (e->button() == Qt::MiddleButton || (e->button() == Qt::LeftButton && e->modifiers() == Qt::ControlModifier)) {
        act->triggerMiddleClick();
        closeAllMenus();
        e->accept();
    }
}

void Menu::closeAllMenus()
{
    QMenu* parentMenu = this;

    while (parentMenu) {
        parentMenu->close();
        parentMenu = qobject_cast<QMenu*>(parentMenu->parentWidget());
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
