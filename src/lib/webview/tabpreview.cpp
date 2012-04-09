/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  Alexander Samilovskih <alexsamilovskih@gmail.com>
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

#include <QFrame>
#include <QLabel>
#include <QPalette>
#include <QStylePainter>
#include <QVBoxLayout>
#include <QStyleOptionFrame>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

TabPreview::TabPreview(QupZilla* mainClass, QWidget* parent)
    : QFrame(parent)
    , p_QupZilla(mainClass)
    , m_pixmap(new QLabel)
    , m_title(new QLabel)
    , m_animationsEnabled(true)
{
    m_pixmap->setAlignment(Qt::AlignHCenter);
    m_title->setAlignment(Qt::AlignHCenter);
    m_title->setWordWrap(true);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(m_pixmap);
    layout->addWidget(m_title);
    layout->setMargin(0);
    layout->setAlignment(Qt::AlignCenter);
    setLayout(layout);

    setBackgroundRole(QPalette::ToolTipBase);
    setForegroundRole(QPalette::ToolTipText);

    setContentsMargins(5, 5, 5, 5);
    setMaximumWidth(250);
    setMaximumHeight(170);

    m_animation =  new QPropertyAnimation(this, "geometry", this);
    m_opacityEffect = new QGraphicsOpacityEffect(this);
    m_opacityAnimation = new QPropertyAnimation(m_opacityEffect, "opacity", this);

    setGraphicsEffect(m_opacityEffect);
    m_opacityEffect->setOpacity(0.0);
}

void TabPreview::setWebTab(WebTab* webTab, bool noPixmap)
{
    if (webTab->isRestored() && !webTab->isLoading() && !noPixmap) {
        m_title->setText(webTab->title());
        m_pixmap->setPixmap(webTab->renderTabPreview());
        m_pixmap->show();
    }
    else {
        m_title->setText(webTab->title());
        m_pixmap->hide();
    }
}

void TabPreview::setAnimationsEnabled(bool enabled)
{
    m_animationsEnabled = enabled;
}

void TabPreview::hideAnimated()
{
    if (m_opacityAnimation->state() == QPropertyAnimation::Running) {
        m_opacityAnimation->stop();
    }

    if (m_animationsEnabled) {
        m_opacityAnimation->setDuration(400);
        m_opacityAnimation->setStartValue(m_opacityEffect->opacity());
        m_opacityAnimation->setEndValue(0.0);
        m_opacityAnimation->start();

        connect(m_opacityAnimation, SIGNAL(finished()), this, SLOT(hide()));
    }
    else {
        QFrame::hide();
    }
}

void TabPreview::hide()
{
    disconnect(m_opacityAnimation, SIGNAL(finished()), this, SLOT(hide()));

    QFrame::hide();
}

void TabPreview::show()
{
    if (!isVisible() && m_animationsEnabled) {
        showAnimated();
    }

    QFrame::show();
}

void TabPreview::showAnimated()
{
    disconnect(m_opacityAnimation, SIGNAL(finished()), this, SLOT(hide()));

    if (m_opacityAnimation->state() == QPropertyAnimation::Running) {
        m_opacityAnimation->stop();
    }
    m_opacityAnimation->setDuration(400);
    m_opacityAnimation->setStartValue(m_opacityEffect->opacity());
    m_opacityAnimation->setEndValue(1.0);
    m_opacityAnimation->start();
}

void TabPreview::paintEvent(QPaintEvent* pe)
{
    QStyleOptionFrame opt;
    opt.init(this);

    QStylePainter painter(this);
    painter.setClipRegion(pe->region());
    painter.drawPrimitive(QStyle::PE_PanelTipLabel, opt);
}

void TabPreview::setFinishingGeometry(const QRect &oldGeometry, const QRect &newGeometry)
{
    m_animation->setDuration(400);
    m_animation->setStartValue(oldGeometry);
    m_animation->setEndValue(newGeometry);
}

QPoint TabPreview::calculatePosition(const QRect &tabRect, const QSize &previewSize)
{
    QPoint p;
    p.setY(tabRect.y() + tabRect.height() + 2);

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

void TabPreview::showOnRect(const QRect &r)
{
    if (m_animation->state() == QPropertyAnimation::Running) {
        m_animation->stop();
    }

    QRect oldGeometry = geometry();
    bool wasVisible = isVisible();

    resize(QSize(250, 170));
    QFrame::show();

    QRect finishingGeometry;

    if (m_pixmap->isVisible()) {
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

    if (!m_animationsEnabled) {
        m_opacityEffect->setOpacity(1.0);
        QFrame::setGeometry(finishingGeometry);
        return;
    }
    else {
        showAnimated();
    }

    if (!wasVisible) {
        oldGeometry = finishingGeometry;
    }

    setFinishingGeometry(oldGeometry, finishingGeometry);
    m_animation->start();

}
