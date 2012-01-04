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
#include "webinspectordockwidget.h"
#include "docktitlebarwidget.h"
#include "webpage.h"
#include "webview.h"
#include "webtab.h"
#include "qupzilla.h"

WebInspectorDockWidget::WebInspectorDockWidget(QupZilla* mainClass)
    : QDockWidget(mainClass)
    , p_QupZilla(mainClass)
{
    setWindowTitle(tr("Web Inspector"));
    setObjectName("WebInspector");
    setFeatures(0);
    setTitleBarWidget(new DockTitleBarWidget(tr("Web Inspector"), this));

    show();
}

void WebInspectorDockWidget::close()
{
    if (qWebKitVersion() == "533.3") {
        delete m_inspector.data();
    }

    p_QupZilla->weView()->webTab()->setInspectorVisible(false);

    hide();
}

void WebInspectorDockWidget::show()
{
    if (!m_inspector.data()) {
        m_inspector = new QWebInspector(this);
        m_inspector.data()->setPage(p_QupZilla->weView()->page());
        setWidget(m_inspector.data());
    }

    if (m_inspector.data()->page() != p_QupZilla->weView()->page()) {
        m_inspector.data()->setPage(p_QupZilla->weView()->page());
    }

    p_QupZilla->weView()->webTab()->setInspectorVisible(true);

    QDockWidget::show();
}

void WebInspectorDockWidget::tabChanged()
{
    if (p_QupZilla->weView()->webTab()->inspectorVisible()) {
        show();
    }
    else {
        close();
    }
}

WebInspectorDockWidget::~WebInspectorDockWidget()
{
}
