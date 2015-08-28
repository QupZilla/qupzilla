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
#include "webinspector.h"
#include "toolbutton.h"
#include "iconprovider.h"

#if QTWEBENGINE_DISABLED

#include <QTimer>

WebInspector::WebInspector(QWidget* parent)
    : QWebInspector(parent)
    , m_closeButton(0)
    , m_blockHideEvent(true)
{
    setObjectName(QSL("web-inspector"));
    setMinimumHeight(80);
}

void WebInspector::updateCloseButton()
{
    if (!m_closeButton) {
        m_closeButton = new ToolButton(this);
        m_closeButton->setAutoRaise(true);
        m_closeButton->setIcon(IconProvider::standardIcon(QStyle::SP_DialogCloseButton));
        connect(m_closeButton, SIGNAL(clicked()), this, SLOT(hideInspector()));
    }

    m_closeButton->show();
    m_closeButton->move(width() - m_closeButton->width(), 0);
}

void WebInspector::hideInspector()
{
    m_blockHideEvent = false;
    hide();
    m_blockHideEvent = true;

    // This is needed to correctly show close button after QWebInspector re-initialization
    m_closeButton->deleteLater();
    m_closeButton = 0;
}

void WebInspector::hideEvent(QHideEvent* event)
{
    // Prevent re-initializing QWebInspector after changing tab
    if (!m_blockHideEvent) {
        QWebInspector::hideEvent(event);
    }
}

void WebInspector::resizeEvent(QResizeEvent* event)
{
    QWebInspector::resizeEvent(event);

    QTimer::singleShot(0, this, SLOT(updateCloseButton()));
}

#endif
