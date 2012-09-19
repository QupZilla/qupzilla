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
#include "enhancedmenu.h"

#include <QMouseEvent>
#include <QApplication>

#ifdef MENUBAR_USE_STATIC_ACTIONS
    QHash<QString, Action*> StaticActionManager::staticMenuActions = QHash<QString, Action*>();
    QHash<QString, bool> StaticActionManager::actionsNoWindowStatePolicy = QHash<QString, bool>();
    QObject* StaticActionManager::m_globalReceiver = 0;
    char* StaticActionManager::m_memberOfGlobalReceiver = 0;
#endif

Menu::Menu(QWidget* parent, bool midButtonAvailable)
    : QMenu(parent)
    , m_midButtonAvailable(midButtonAvailable)
{
}

Menu::Menu(const QString &title, QWidget* parent, bool midButtonAvailable)
    : QMenu(title, parent)
    , m_midButtonAvailable(midButtonAvailable)
{
}

QAction *Menu::addAction(QAction *action)
{
    QMenu::addAction(action);
    return action;
}
    
Action *Menu::addAction(Action *action, const QString &key, bool noWindowState)
{
#ifdef MENUBAR_USE_STATIC_ACTIONS
    if (!key.isEmpty()) {
        StaticActionManager::insertActionToHash(key, action, noWindowState);
    }
#endif
    QMenu::addAction(action);
    return action;
}

Action *Menu::addAction(const QString &text, const QString &key, bool noWindowState)
{
    Action *action = StaticActionManager::actionInstance(key, text, QIcon(), noWindowState, this);
    addAction(action);
    return action;
}

Action *Menu::addAction(const QIcon &icon, const QString &text, const QString &key, bool noWindowState)
{
    Action *action =  StaticActionManager::actionInstance(key, text, icon, noWindowState, this);
    addAction(action);
    return action;
}

Action *Menu::addAction(const QString &text, const QObject *receiver, const char* member,
                        const QString &key, bool noWindowState, const QKeySequence &shortcut)
{
    Action *action = StaticActionManager::actionInstance(key, text, QIcon(), noWindowState, this);
#ifdef QT_NO_SHORTCUT
    Q_UNUSED(shortcut);
#else
    action->setShortcut(shortcut);
#endif

#ifdef MENUBAR_USE_STATIC_ACTIONS
    Q_UNUSED(receiver);
    Q_UNUSED(member);
#else
    QObject::connect(action, SIGNAL(triggered(bool)), receiver, member);
#endif
    addAction(action);
    return action;
}

Action *Menu::addAction(const QIcon &icon, const QString &text, const QObject *receiver,
                          const char* member, const QKeySequence &shortcut, const QString &key, bool noWindowState)
{
    return addAction(icon, text, receiver, member, key, noWindowState, shortcut);
}

Action *Menu::addAction(const QIcon &icon, const QString &text, const QObject *receiver,
                        const char *member, const QString &key, bool noWindowState, const QKeySequence &shortcut)
{
    Action *action = StaticActionManager::actionInstance(key, text, icon, noWindowState, this);
#ifdef QT_NO_SHORTCUT
    Q_UNUSED(shortcut);
#else
    action->setShortcut(shortcut);
#endif

#ifdef MENUBAR_USE_STATIC_ACTIONS
    Q_UNUSED(receiver);
    Q_UNUSED(member);
#else
    QObject::connect(action, SIGNAL(triggered(bool)), receiver, member);
#endif
    addAction(action);
    return action;
}

void Menu::mouseReleaseEvent(QMouseEvent* e)
{
    if (!m_midButtonAvailable) {
        QMenu::mouseReleaseEvent(e);
        return;
    }
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


void StaticActionManager::setGlobalReciver(QObject *receiver, char *member)
{
#ifdef MENUBAR_USE_STATIC_ACTIONS
    disconnectFromGlobalReceiver(m_globalReceiver, m_memberOfGlobalReceiver);
    m_globalReceiver = receiver;
    m_memberOfGlobalReceiver = member;
    connectToGlobalReceiver(m_globalReceiver, m_memberOfGlobalReceiver);
#endif
}

#ifdef MENUBAR_USE_STATIC_ACTIONS
Action *StaticActionManager::getActionByKey(const QString &key, bool *noWindowState)
{
    // custom assert failure
    if (!staticMenuActions.contains(key)) {
        qFatal("Failure when getting action object, the key doesn't exist> key: \"%s\", file: %s, line: %d", key.toLatin1().data(), __FILE__, __LINE__);
    }

    if (noWindowState) {
        *noWindowState = actionsNoWindowStatePolicy.value(key, false);
    }
    return staticMenuActions.value(key);
}

void StaticActionManager::insertActionToHash(const QString &key, Action *action, bool noWindowState)
{
    actionsNoWindowStatePolicy.insert(key, noWindowState);
    staticMenuActions.insert(key, action);
}

void StaticActionManager::connectToGlobalReceiver(const QObject *receiver, const char *member)
{
    QHash<QString, Action*>::const_iterator i = staticMenuActions.constBegin();
    while (i != staticMenuActions.constEnd()) {
        QObject::connect(i.value(), SIGNAL(triggered(bool)), receiver, member);
        ++i;
    }
}

void StaticActionManager::disconnectFromGlobalReceiver(const QObject *receiver, const char *member)
{
    if (!receiver || !member) {
        return;
    }

    QHash<QString, Action*>::const_iterator i = staticMenuActions.constBegin();
    while (i != staticMenuActions.constEnd()) {
        QObject::disconnect(i.value(), SIGNAL(triggered(bool)), receiver, member);
        ++i;
    }
}
#endif

void StaticActionManager::enableStaticActions(bool allActions)
{
#ifdef MENUBAR_USE_STATIC_ACTIONS
    QHash<QString, Action*>::const_iterator i = staticMenuActions.constBegin();
    while (i != staticMenuActions.constEnd()) {
        bool newState = true;
        if (!allActions) {
            newState = actionsNoWindowStatePolicy.value(i.key(), false);
        }

        i.value()->setEnabled(newState);
        ++i;
    }
#endif
}

Action *StaticActionManager::actionInstance(const QString &key, const QString &displayName, const QIcon &icon, bool noWindowState, QObject *parent)
{
    Action *action = 0;
#ifdef MENUBAR_USE_STATIC_ACTIONS
    if (key.isEmpty()) {
        action = new Action(displayName, 0);

        if (!icon.isNull()) {
            action->setIcon(icon);
        }
        return action;
    }

    if (key.contains(QLatin1String("separator"), Qt::CaseInsensitive) ) {
        action = new Action(0);
        action->setSeparator(true);
        return action;
    }

    action = staticMenuActions.value(key, 0);
    if (!action) {
        action = new Action(displayName, 0);

        if (!icon.isNull()) {
            action->setIcon(icon);
        }
        action->setObjectName(key);
        insertActionToHash(key, action, noWindowState);
    }

    if (m_globalReceiver && m_memberOfGlobalReceiver) {
        connect(action, SIGNAL(triggered(bool)), m_globalReceiver, m_memberOfGlobalReceiver);
    }
#else
    action = new Action(displayName, parent);

    if (!icon.isNull()) {
        action->setIcon(icon);
    }
#endif
    return action;
}
