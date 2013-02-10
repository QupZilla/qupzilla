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
#include "autofillicon.h"
#include "autofillwidget.h"

#include <QContextMenuEvent>

AutoFillIcon::AutoFillIcon(QWidget* parent)
    : ClickableLabel(parent)
    , m_view(0)
{
    setObjectName("locationbar-autofillicon");
    setCursor(Qt::PointingHandCursor);
    setToolTip(AutoFillWidget::tr("Choose username to login"));
    setFocusPolicy(Qt::ClickFocus);

    connect(this, SIGNAL(clicked(QPoint)), this, SLOT(iconClicked()));
}

void AutoFillIcon::setWebView(WebView* view)
{
    m_view = view;
}

void AutoFillIcon::setFormData(const QList<AutoFillData> &data)
{
    m_data = data;
}

void AutoFillIcon::iconClicked()
{
    if (!m_view) {
        return;
    }

    AutoFillWidget* widget = new AutoFillWidget(m_view, this);
    widget->setFormData(m_data);
    widget->showAt(parentWidget());
}

void AutoFillIcon::contextMenuEvent(QContextMenuEvent* ev)
{
    // Prevent propagating to LocationBar
    ev->accept();
}

void AutoFillIcon::mousePressEvent(QMouseEvent* ev)
{
    ClickableLabel::mousePressEvent(ev);

    // Prevent propagating to LocationBar
    ev->accept();
}
