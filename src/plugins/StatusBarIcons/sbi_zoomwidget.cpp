/* ============================================================
* StatusBarIcons - Extra icons in statusbar for QupZilla
* Copyright (C) 2014  David Rosca <nowrep@gmail.com>
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
#include "sbi_zoomwidget.h"
#include "tabbedwebview.h"
#include "browserwindow.h"
#include "tabwidget.h"

SBI_ZoomWidget::SBI_ZoomWidget(BrowserWindow* parent)
    : QSlider(parent)
    , m_window(parent)
{
    setOrientation(Qt::Horizontal);
    setFixedWidth(100);
    setMaximumHeight(20);

    setPageStep(2);
    setSingleStep(1);
    setRange(0, WebView::zoomLevels().count() - 1);

    connect(this, SIGNAL(valueChanged(int)), this, SLOT(valueChanged(int)));
    connect(m_window->tabWidget(), SIGNAL(currentChanged(int)), this, SLOT(currentViewChanged()));

    currentViewChanged();
}

void SBI_ZoomWidget::valueChanged(int value)
{
    TabbedWebView* view = m_window->weView();

    if (view) {
        view->setZoomLevel(value);
        setToolTip(tr("Zoom: %1%").arg(view->zoomFactor() * 100));
    }
}

void SBI_ZoomWidget::currentViewChanged()
{
    TabbedWebView* view = m_window->weView();

    if (view) {
        connect(view, SIGNAL(zoomLevelChanged(int)), this, SLOT(setValue(int)));
        setValue(view->zoomLevel());
    }
}
