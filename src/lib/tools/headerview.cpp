/* ============================================================
* QupZilla - WebKit based browser
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
#include "headerview.h"

#include <QMenu>
#include <QContextMenuEvent>

HeaderView::HeaderView(QAbstractItemView* parent)
    : QHeaderView(Qt::Horizontal, parent)
    , m_parent(parent)
    , m_menu(0)
    , m_resizeOnShow(false)
{
    setSectionsMovable(true);
    setStretchLastSection(true);
    setDefaultAlignment(Qt::AlignLeft);
    setMinimumSectionSize(60);
}

void HeaderView::setDefaultSectionSizes(const QList<double> &sizes)
{
    m_sectionSizes = sizes;
}

QList<double> HeaderView::defaultSectionSizes() const
{
    return m_sectionSizes;
}

bool HeaderView::restoreState(const QByteArray &state)
{
    m_resizeOnShow = !QHeaderView::restoreState(state);

    return !m_resizeOnShow;
}

void HeaderView::showEvent(QShowEvent* event)
{
    if (m_resizeOnShow) {
        for (int i = 0; i < m_sectionSizes.count(); ++i) {
            int size = m_parent->width() * m_sectionSizes.at(i);
            resizeSection(i, size);
        }
    }

    QHeaderView::showEvent(event);
}

void HeaderView::contextMenuEvent(QContextMenuEvent* event)
{
    if (!m_menu) {
        m_menu = new QMenu(this);

        for (int i = 0; i < count(); ++i) {
            QAction* act = new QAction(model()->headerData(i, Qt::Horizontal).toString(), m_menu);
            act->setCheckable(true);
            act->setData(i);

            connect(act, SIGNAL(triggered()), this, SLOT(toggleSectionVisibility()));
            m_menu->addAction(act);
        }
    }

    for (int i = 0; i < m_menu->actions().count(); ++i) {
        QAction* act = m_menu->actions().at(i);
        act->setEnabled(i > 0);
        act->setChecked(!isSectionHidden(i));
    }

    m_menu->popup(event->globalPos());
}

void HeaderView::toggleSectionVisibility()
{
    if (QAction* act = qobject_cast<QAction*>(sender())) {
        int index = act->data().toInt();

        setSectionHidden(index, !isSectionHidden(index));
    }
}
