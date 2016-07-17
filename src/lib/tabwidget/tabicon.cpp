/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2014-2016  David Rosca <nowrep@gmail.com>
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
#include "tabicon.h"
#include "webtab.h"
#include "iconprovider.h"
#include "tabbedwebview.h"

#include <QTimer>

#define ANIMATION_INTERVAL 70

TabIcon::TabIcon(QWidget* parent)
    : QWidget(parent)
    , m_tab(0)
    , m_currentFrame(0)
    , m_animationRunning(false)
{
    setObjectName(QSL("tab-icon"));

    m_animationPixmap = QIcon(QSL(":icons/other/loading.png")).pixmap(288, 16);
    m_framesCount = m_animationPixmap.width() / m_animationPixmap.height();

    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(ANIMATION_INTERVAL);
    connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(updateAnimationFrame()));

    resize(16, 16);

    setIcon(IconProvider::emptyWebIcon());
}

void TabIcon::setWebTab(WebTab* tab)
{
    m_tab = tab;

    connect(m_tab->webView(), SIGNAL(loadStarted()), this, SLOT(showLoadingAnimation()));
    connect(m_tab->webView(), SIGNAL(loadFinished(bool)), this, SLOT(hideLoadingAnimation()));
    connect(m_tab->webView(), SIGNAL(iconChanged()), this, SLOT(showIcon()));

    showIcon();
}

void TabIcon::setIcon(const QIcon &icon)
{
    m_sitePixmap = icon.pixmap(16);
    update();
}

void TabIcon::showLoadingAnimation()
{
    m_currentFrame = 0;

    updateAnimationFrame();
}

void TabIcon::hideLoadingAnimation()
{
    m_animationRunning = false;

    m_updateTimer->stop();
    showIcon();
}

void TabIcon::showIcon()
{
    m_sitePixmap = m_tab->icon().pixmap(16);
    update();
}

void TabIcon::updateAnimationFrame()
{
    if (!m_animationRunning) {
        m_updateTimer->start();
        m_animationRunning = true;
    }

    update();
    m_currentFrame = (m_currentFrame + 1) % m_framesCount;
}

void TabIcon::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter p(this);

    const int size = 16;
    const int pixmapSize = qRound(size * m_animationPixmap.devicePixelRatioF());

    // Center the pixmap in rect
    QRect r = rect();
    r.setX((r.width() - size) / 2);
    r.setY((r.height() - size) / 2);
    r.setWidth(size);
    r.setHeight(size);

    if (m_animationRunning)
        p.drawPixmap(r, m_animationPixmap, QRect(m_currentFrame * pixmapSize, 0, pixmapSize, pixmapSize));
    else
        p.drawPixmap(r, m_sitePixmap);
}
