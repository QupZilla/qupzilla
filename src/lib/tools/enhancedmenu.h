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
#ifndef ENHANCEDMENU_H
#define ENHANCEDMENU_H

#include <QMenu>

#include "qz_namespace.h"

class Action;

class QT_QUPZILLA_EXPORT Menu : public QMenu
{
    Q_OBJECT
public:
    explicit Menu(QWidget* parent = 0, bool midButtonAvailable = true);
    explicit Menu(const QString &title, QWidget* parent = 0, bool midButtonAvailable = true);

    QAction *addAction(QAction *action);
    Action *addAction(Action *action, const QString &key/* = QString()*/, bool noWindowState = false);
    Action *addAction(const QString &text, const QString &key = QString(), bool noWindowState = false);
    Action *addAction(const QIcon &icon, const QString &text, const QString &key = QString(), bool noWindowState = false);
    Action *addAction(const QString &text, const QObject *receiver, const char* member,
                      const QString &key = QString(), bool noWindowState = false, const QKeySequence &shortcut = 0);

    Action *addAction(const QIcon &icon, const QString &text,
                      const QObject *receiver, const char* member,
                      const QKeySequence &shortcut = 0, const QString &key = QString(), bool noWindowState = false);

    Action *addAction(const QIcon &icon, const QString &text,
                      const QObject *receiver, const char* member,
                      const QString &key = QString(), bool noWindowState = false, const QKeySequence &shortcut = 0);

signals:
    void menuMiddleClicked(Menu*);

public slots:

private:
    void mouseReleaseEvent(QMouseEvent* e);
    void closeAllMenus();

    bool m_midButtonAvailable;
};

class QT_QUPZILLA_EXPORT Action : public QAction
{
    Q_OBJECT
public:
    explicit Action(QObject* parent = 0);
    explicit Action(const QString &text, QObject* parent = 0);
    explicit Action(const QIcon &icon, const QString &text, QObject* parent = 0);

signals:
    void middleClicked();

public slots:
    void triggerMiddleClick();

};

#ifdef MENUBAR_USE_STATIC_ACTIONS
#define GET_ACTION StaticActionManager::getActionByKey
#endif

class QT_QUPZILLA_EXPORT StaticActionManager : public QObject
{
    Q_OBJECT
public:
    static void setGlobalReciver(QObject* receiver = 0, char* member = 0);
    static void enableStaticActions(bool allActions);
    static Action* actionInstance(const QString &key, const QString &displayName = QString(),
                            const QIcon &icon = QIcon(), bool noWindowState = false, QObject* parent = 0); 
#ifdef MENUBAR_USE_STATIC_ACTIONS
    static Action* getActionByKey(const QString &key, bool* noWindowState = 0);
    static void insertActionToHash(const QString &key, Action* action, bool noWindowState = false);

private:
    static void connectToGlobalReceiver(const QObject* receiver, const char* member);
    static void disconnectFromGlobalReceiver(const QObject* receiver, const char* member);

    static QObject* m_globalReceiver;
    static char* m_memberOfGlobalReceiver;

    static QHash<QString, bool> actionsNoWindowStatePolicy;
    static QHash<QString, Action*> staticMenuActions;
#endif
};

#endif // ENHANCEDMENU_H
