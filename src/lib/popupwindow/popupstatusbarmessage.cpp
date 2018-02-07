/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018 David Rosca <nowrep@gmail.com>
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
#include "popupstatusbarmessage.h"
#include "popupwindow.h"
#include "popupwebview.h"
#include "webpage.h"
#include "mainapplication.h"

#include <QStatusBar>
#include <QStyle>

PopupStatusBarMessage::PopupStatusBarMessage(PopupWindow* window)
    : m_popupWindow(window)
    , m_statusBarText(new TipLabel(window))
{
}

void PopupStatusBarMessage::showMessage(const QString &message)
{
    if (m_popupWindow->statusBar()->isVisible()) {
        m_popupWindow->statusBar()->showMessage(message);
    }
#ifdef Q_OS_WIN
    else if (mApp->activeWindow() == m_popupWindow) {
#else
    else {
#endif
        PopupWebView* view = m_popupWindow->webView();

        m_statusBarText->setText(message);
        m_statusBarText->setMaximumWidth(view->width());
        m_statusBarText->resize(m_statusBarText->sizeHint());

        QPoint position;
        position.setY(view->height() - m_statusBarText->height());

        QRect statusRect = QRect(view->mapToGlobal(QPoint(0, position.y())), m_statusBarText->size());

        if (statusRect.contains(QCursor::pos())) {
            position.setY(position.y() - m_statusBarText->height());
        }

        m_statusBarText->move(view->mapToGlobal(position));
        m_statusBarText->show(view);
    }
}

void PopupStatusBarMessage::clearMessage()
{
    if (m_popupWindow->statusBar()->isVisible()) {
        m_popupWindow->statusBar()->showMessage(QString());
    }
    else {
        m_statusBarText->hideDelayed();
    }
}
