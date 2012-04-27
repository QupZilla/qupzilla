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
#include "locationcompleter.h"
#include "locationcompletermodel.h"
#include "locationcompleterview.h"
#include "locationcompleterdelegate.h"
#include "locationbar.h"

LocationCompleterView* LocationCompleter::s_view = 0;
LocationCompleterModel* LocationCompleter::s_model = 0;

LocationCompleter::LocationCompleter(QObject* parent)
    : QObject(parent)
    , m_locationBar(0)
{
    if (!s_view) {
        s_model = new LocationCompleterModel;
        s_view = new LocationCompleterView;

        s_view->setModel(s_model);
        s_view->setItemDelegate(new LocationCompleterDelegate(s_view));
    }
}

void LocationCompleter::setLocationBar(LocationBar* locationBar)
{
    m_locationBar = locationBar;
}

void LocationCompleter::closePopup()
{
    s_view->close();
}

void LocationCompleter::complete(const QString &string)
{
    s_model->refreshCompletions(string);

    showPopup();
}

void LocationCompleter::showMostVisited()
{
    s_model->refreshCompletions(QString());

    showPopup();
}

void LocationCompleter::currentChanged(const QModelIndex &index)
{
    QString completion = index.data().toString();
    if (completion.isEmpty()) {
        completion = m_originalText;
    }

    emit showCompletion(completion);
}

void LocationCompleter::popupClosed()
{
    disconnect(s_view->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), this, SLOT(currentChanged(QModelIndex)));
    disconnect(s_view, SIGNAL(clicked(QModelIndex)), this, SIGNAL(completionActivated()));
    disconnect(s_view, SIGNAL(closed()), this, SLOT(popupClosed()));
}

void LocationCompleter::showPopup()
{
    Q_ASSERT(m_locationBar);

    if (s_model->rowCount() == 0) {
        s_view->close();
        return;
    }

    if (s_view->isVisible()) {
        adjustPopupSize();
        return;
    }

    QRect popupRect(m_locationBar->mapToGlobal(m_locationBar->pos()), m_locationBar->size());
    popupRect.setY(popupRect.bottom());

    s_view->setFocusProxy(m_locationBar);
    s_view->setGeometry(popupRect);

    connect(s_view->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), this, SLOT(currentChanged(QModelIndex)));
    connect(s_view, SIGNAL(clicked(QModelIndex)), this, SIGNAL(completionActivated()));
    connect(s_view, SIGNAL(closed()), this, SLOT(popupClosed()));

    adjustPopupSize();
}

void LocationCompleter::adjustPopupSize()
{
    const int maxItemsCount = 6;

    int popupHeight = s_view->sizeHintForRow(0) * qMin(maxItemsCount, s_model->rowCount());
    popupHeight += 2 * s_view->frameWidth();

    s_view->resize(s_view->width(), popupHeight);
    s_view->setCurrentIndex(QModelIndex());
    s_view->show();

    m_originalText = m_locationBar->text();
}
