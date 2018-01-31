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
#include "siteicon.h"
#include "siteinfowidget.h"
#include "locationbar.h"
#include "tabbedwebview.h"
#include "qztools.h"
#include "siteinfo.h"

#include <QDrag>
#include <QTimer>
#include <QMimeData>
#include <QApplication>
#include <QContextMenuEvent>

SiteIcon::SiteIcon(LocationBar *parent)
    : ToolButton(parent)
    , m_window(nullptr)
    , m_locationBar(parent)
    , m_view(0)
{
    setObjectName("locationbar-siteicon");
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    setCursor(Qt::ArrowCursor);
    setToolTip(LocationBar::tr("Show information about this page"));
    setFocusPolicy(Qt::NoFocus);

    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(100);
    m_updateTimer->setSingleShot(true);
    connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(updateIcon()));
}

void SiteIcon::setBrowserWindow(BrowserWindow *window)
{
    m_window = window;
}

void SiteIcon::setWebView(WebView* view)
{
    m_view = view;
}

void SiteIcon::setIcon(const QIcon &icon)
{
    bool wasNull = m_icon.isNull();

    m_icon = icon;

    if (wasNull) {
        updateIcon();
    }
    else {
        m_updateTimer->start();
    }
}

void SiteIcon::updateIcon()
{
    ToolButton::setIcon(m_icon);
}

void SiteIcon::popupClosed()
{
    setDown(false);
}

void SiteIcon::contextMenuEvent(QContextMenuEvent* e)
{
    // Prevent propagating to LocationBar
    e->accept();
}

void SiteIcon::mousePressEvent(QMouseEvent* e)
{
    if (e->buttons() == Qt::LeftButton) {
        m_dragStartPosition = e->pos();
    }

    // Prevent propagating to LocationBar
    e->accept();

    ToolButton::mousePressEvent(e);
}

void SiteIcon::mouseReleaseEvent(QMouseEvent* e)
{
    // Mouse release event is restoring Down state
    // So we pause updates to prevent flicker

    bool activated = false;

    if (e->button() == Qt::LeftButton && rect().contains(e->pos())) {
        // Popup may not be always shown, eg. on qupzilla: pages
        activated = showPopup();
    }

    if (activated) {
        setUpdatesEnabled(false);
    }

    ToolButton::mouseReleaseEvent(e);

    if (activated) {
        setDown(true);
        setUpdatesEnabled(true);
    }
}

void SiteIcon::mouseMoveEvent(QMouseEvent* e)
{
    if (!m_locationBar || e->buttons() != Qt::LeftButton) {
        ToolButton::mouseMoveEvent(e);
        return;
    }

    int manhattanLength = (e->pos() - m_dragStartPosition).manhattanLength();
    if (manhattanLength <= QApplication::startDragDistance()) {
        ToolButton::mouseMoveEvent(e);
        return;
    }

    const QUrl url = m_locationBar->webView()->url();
    const QString title = m_locationBar->webView()->title();

    if (url.isEmpty() || title.isEmpty()) {
        ToolButton::mouseMoveEvent(e);
        return;
    }

    QDrag* drag = new QDrag(this);
    QMimeData* mime = new QMimeData;
    mime->setUrls(QList<QUrl>() << url);
    mime->setText(title);
    mime->setImageData(icon().pixmap(16).toImage());

    drag->setMimeData(mime);
    drag->setPixmap(QzTools::createPixmapForSite(icon(), title, url.toString()));
    drag->exec();

    // Restore Down state
    setDown(false);
}

bool SiteIcon::showPopup()
{
    if (!m_view || !m_window) {
        return false;
    }

    QUrl url = m_view->url();

    if (!SiteInfo::canShowSiteInfo(url))
        return false;

    setDown(true);

    SiteInfoWidget* info = new SiteInfoWidget(m_window);
    info->showAt(parentWidget());

    connect(info, SIGNAL(destroyed()), this, SLOT(popupClosed()));

    return true;
}
