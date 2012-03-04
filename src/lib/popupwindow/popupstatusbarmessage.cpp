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
#include "popupstatusbarmessage.h"
#include "popupwindow.h"
#include "popupwebview.h"
#include "statusbarmessage.h"
#include "mainapplication.h"

#include <QStatusBar>
#include <QStyle>
#include <QWebFrame>

PopupStatusBarMessage::PopupStatusBarMessage(PopupWindow* mainClass)
    : m_popupWindow(mainClass)
    , m_statusBarText(new TipLabel(mainClass))
{
}

void PopupStatusBarMessage::showMessage(const QString &message)
{
    if (m_popupWindow->statusBar()->isVisible()) {
        m_popupWindow->statusBar()->showMessage(message);
    }
#ifdef Q_WS_WIN
    else if (mApp->activeWindow() == m_popupWindow) {
#else
    else {
#endif
        PopupWebView* view = m_popupWindow->webView();
        QWebFrame* mainFrame = view->page()->mainFrame();

        int horizontalScrollSize = 0;
        int verticalScrollSize = 0;
        const int scrollbarSize = m_popupWindow->style()->pixelMetric(QStyle::PM_ScrollBarExtent);

        if (mainFrame->scrollBarMaximum(Qt::Horizontal)) {
            horizontalScrollSize = scrollbarSize;
        }
        if (mainFrame->scrollBarMaximum(Qt::Vertical)) {
            verticalScrollSize = scrollbarSize;
        }

        m_statusBarText->setText(message);
        m_statusBarText->setMaximumWidth(view->width() - verticalScrollSize);
        m_statusBarText->resize(m_statusBarText->sizeHint());

        QPoint position;
        position.setY(view->height() - horizontalScrollSize - m_statusBarText->height());

        QRect statusRect = QRect(view->mapToGlobal(QPoint(0, position.y())), m_statusBarText->size());

        if (statusRect.contains(QCursor::pos())) {
            position.setY(position.y() - m_statusBarText->height());
        }

        m_statusBarText->move(view->mapToGlobal(position));
        m_statusBarText->show();
    }
}

void PopupStatusBarMessage::clearMessage()
{
    if (m_popupWindow->statusBar()->isVisible()) {
        m_popupWindow->statusBar()->showMessage("");
    }
    else {
        m_statusBarText->hide();
    }
}
