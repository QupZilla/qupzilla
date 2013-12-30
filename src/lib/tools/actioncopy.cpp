/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2013  David Rosca <nowrep@gmail.com>
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
#include "actioncopy.h"

ActionCopy::ActionCopy(QAction* original, QObject* parent)
    : QAction(parent)
    , m_action(original)
{
    updateAction();

    connect(m_action, SIGNAL(changed()), this, SLOT(updateAction()));
    connect(this, SIGNAL(toggled(bool)), this, SLOT(actionToggled()));
    connect(this, SIGNAL(triggered()), this, SLOT(actionTriggered()));
}

void ActionCopy::updateAction()
{
    const QString shortcutString = m_action->shortcut().toString(QKeySequence::NativeText);
    const QString actionText = QString("%1\t%2").arg(m_action->text(), shortcutString);

    setText(actionText);
    setIcon(m_action->icon());

}

void ActionCopy::actionToggled()
{
    m_action->toggle();
}

void ActionCopy::actionTriggered()
{
    m_action->trigger();
}
