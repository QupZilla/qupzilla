/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2013  Alexander Samilovskih <alexsamilovskih@gmail.com>
*                          David Rosca <nowrep@gmail.com>
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
#include "qupzilla.h"
#include "tabpreview.h"
#include "webtab.h"
#include "tabbedwebview.h"

#include <QLabel>
#include <QPaintEvent>
#include <QPalette>
#include <QStylePainter>
#include <QVBoxLayout>
#include <QStyleOptionFrame>

TabPreview::TabPreview(QupZilla* mainClass, QWidget* parent)
    : QFrame(parent)
    , p_QupZilla(mainClass)
    , m_previewIndex(-1)
    , m_animationsEnabled(true)
    , m_stepX(0)
    , m_stepY(0)
    , m_stepWidth(0)
    , m_stepHeight(0)
{
    m_pixmapLabel = new QLabel(this);
    m_pixmapLabel->setAlignment(Qt::AlignHCenter);

    m_title = new QLabel(this);
    m_title->setAlignment(Qt::AlignHCenter);
    m_title->setWordWrap(true);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(m_pixmapLabel);
    layout->addWidget(m_title);
    layout->setMargin(0);
    layout->setAlignment(Qt::AlignCenter);
    setLayout(layout);

    setBackgroundRole(QPalette::ToolTipBase);
    setForegroundRole(QPalette::ToolTipText);

    setContentsMargins(5, 5, 5, 5);
    setMaximumWidth(250);
    setMaximumHeight(170);

#ifdef ENABLE_OPACITY_EFFECT
    setGraphicsEffect(&m_opacityEffect);
    m_opacityEffect.setOpacity(0.0);
    connect(&m_opacityTimeLine, SIGNAL(frameChanged(int)), this, SLOT(setOpacity(int)));
#endif

    m_animation.setDuration(400);
    m_animation.setFrameRange(0, 100);
    m_animation.setUpdateInterval(20); // 50 fps
    connect(&m_animation, SIGNAL(frameChanged(int)), this, SLOT(setAnimationFrame(int)));
}

int TabPreview::previewIndex()
{
    return m_previewIndex;
}

void TabPreview::setPreviewIndex(int index)
{
    m_previewIndex = index;
}

void TabPreview::setWebTab(WebTab* webTab, bool noPixmap)
{
    if (webTab->isRestored() && !webTab->isLoading() && !noPixmap) {
        m_title->setText(webTab->title());
        m_pixmapLabel->setPixmap(webTab->renderTabPreview());
        m_pixmapLabel->show();
    }
    else {
        m_title->setText(webTab->title());
        m_pixmapLabel->hide();
    }
}

void TabPreview::setAnimationsEnabled(bool enabled)
{
    m_animationsEnabled = enabled;
}

void TabPreview::hideAnimated()
{
#ifdef ENABLE_OPACITY_EFFECT
    if (m_opacityTimeLine.state() == QTimeLine::Running) {
        m_opacityTimeLine.stop();
    }

    if (m_animationsEnabled) {
        m_opacityTimeLine.setDuration(400);
        m_opacityTimeLine.setStartFrame(m_opacityEffect.opacity() * 100);
        m_opacityTimeLine.setEndFrame(0);
        m_opacityTimeLine.start();

        connect(&m_opacityTimeLine, SIGNAL(finished()), this, SLOT(hide()));
    }
    else
#endif
    {
        QFrame::hide();
    }
}

void TabPreview::hide()
{
    m_previewIndex = -1;
#ifdef ENABLE_OPACITY_EFFECT
    disconnect(&m_opacityTimeLine, SIGNAL(finished()), this, SLOT(hide()));
#endif

    QFrame::hide();
}

void TabPreview::show()
{
    if (!isVisible() && m_animationsEnabled) {
        showAnimated();
    }

    QFrame::show();
}

void TabPreview::showOnRect(const QRect &r)
{
    if (m_animation.state() == QTimeLine::Running) {
        m_animation.stop();
    }

    m_startGeometry = geometry();
    bool wasVisible = isVisible();
    QRect finishingGeometry;

    resize(QSize(250, 170));
    QFrame::show();

    if (m_pixmapLabel->isVisible()) {
        m_title->setWordWrap(false);
        m_title->setText(m_title->fontMetrics().elidedText(m_title->text(), Qt::ElideRight, 240));

        QSize previewSize(250, 170);
        finishingGeometry = QRect(calculatePosition(r, previewSize), previewSize);
    }
    else {
        m_title->setWordWrap(true);

        QSize previewSize = sizeHint();
        previewSize.setWidth(qMin(previewSize.width() + 2 * 5, 240));
        previewSize.setHeight(qMin(previewSize.height() + 2 * 5, 130));

        finishingGeometry = QRect(calculatePosition(r, previewSize), previewSize);
    }

#ifdef ENABLE_OPACITY_EFFECT
    if (!m_animationsEnabled) {
        m_opacityEffect.setOpacity(1.0);
#else
    if (!m_animationsEnabled || !wasVisible) {
#endif
        QFrame::setGeometry(finishingGeometry);
        return;
    }
    else {
        showAnimated();
    }

    if (!wasVisible) {
        m_startGeometry = finishingGeometry;
    }

    QFrame::setGeometry(m_startGeometry);

    calculateSteps(m_startGeometry, finishingGeometry);
    m_animation.start();
}

#ifdef ENABLE_OPACITY_EFFECT
void TabPreview::setOpacity(int opacity)
{
    m_opacityEffect.setOpacity(opacity / 100.0);
}
#endif

void TabPreview::setAnimationFrame(int frame)
{
    QRect g;
    g.setX(m_startGeometry.x() + frame * m_stepX);
    g.setY(m_startGeometry.y() + frame * m_stepY);
    g.setWidth(m_startGeometry.width() + frame * m_stepWidth);
    g.setHeight(m_startGeometry.height() + frame * m_stepHeight);

    setGeometry(g);
}

void TabPreview::showAnimated()
{
#ifdef ENABLE_OPACITY_EFFECT
    disconnect(&m_opacityTimeLine, SIGNAL(finished()), this, SLOT(hide()));

    if (m_opacityTimeLine.state() == QTimeLine::Running) {
        m_opacityTimeLine.stop();
    }

    m_opacityTimeLine.setDuration(400);
    m_opacityTimeLine.setStartFrame(m_opacityEffect.opacity() * 100);
    m_opacityTimeLine.setEndFrame(100);
    m_opacityTimeLine.start();
#endif
}

void TabPreview::paintEvent(QPaintEvent* pe)
{
    QStyleOptionFrame opt;
    opt.init(this);

    QStylePainter painter(this);
    painter.setClipRegion(pe->region());
    painter.drawPrimitive(QStyle::PE_PanelTipLabel, opt);
}

void TabPreview::calculateSteps(const QRect &oldGeometry, const QRect &newGeometry)
{
    m_stepX = (newGeometry.x() - oldGeometry.x()) / 100.0;
    m_stepY = (newGeometry.y() - oldGeometry.y()) / 100.0;
    m_stepWidth = (newGeometry.width() - oldGeometry.width()) / 100.0;
    m_stepHeight = (newGeometry.height() - oldGeometry.height()) / 100.0;
}

QPoint TabPreview::calculatePosition(const QRect &tabRect, const QSize &previewSize)
{
    QPoint p;
    p.setY(tabRect.y() + tabRect.height() + 1);

    // Map to center of tab
    if (tabRect.width() > previewSize.width()) {
        int extraWidth = tabRect.width() - previewSize.width();
        p.setX(tabRect.x() + extraWidth / 2);
    }
    else {
        int extraWidth = previewSize.width() - tabRect.width();
        p.setX(tabRect.x() - extraWidth / 2);
    }

    // Ensure the whole preview is always shown
    if (p.x() < 0) {
        p.setX(0);
    }
    if (p.x() + previewSize.width() > p_QupZilla->width()) {
        int extraWidth = p.x() + previewSize.width() - p_QupZilla->width();
        p.setX(p.x() - extraWidth);
    }

    return p;
}
