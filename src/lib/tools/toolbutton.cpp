/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2013  David Rosca <nowrep@gmail.com>
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
#include "toolbutton.h"

#include <QMouseEvent>
#include <QMenu>
#include <QPainter>
#include <QApplication>
#include <QStyleOptionToolButton>

ToolButton::ToolButton(QWidget* parent)
    : QToolButton(parent)
    , m_usingMultiIcon(false)
    , m_showMenuInside(false)
    , m_forceHidden(false)
{
    setMinimumWidth(16);
}

void ToolButton::setThemeIcon(const QString &image)
{
    m_themeIcon = image;
    setIcon(QIcon::fromTheme(image));
    m_usingMultiIcon = false;
}

void ToolButton::setFallbackIcon(const QIcon &image)
{
    if (icon().isNull()) {
        setIcon(image);
        m_usingMultiIcon = false;
    }
}

void ToolButton::setIcon(const QIcon &image)
{
    if (m_usingMultiIcon) {
        setFixedSize(sizeHint());
    }
    m_usingMultiIcon = false;

    QToolButton::setIcon(image);
}

void ToolButton::setShowMenuInside(bool inside)
{
    m_showMenuInside = inside;
}

bool ToolButton::showMenuInside() const
{
    return m_showMenuInside;
}

void ToolButton::setVisible(bool visible)
{
    QToolButton::setVisible(!m_forceHidden && visible);
}

bool ToolButton::isForceHidden()
{
    return m_forceHidden;
}

void ToolButton::setForceHidden(bool enable)
{
    m_forceHidden = enable;
    if (m_forceHidden) {
        hide();
    }
}

void ToolButton::setData(const QVariant &data)
{
    m_data = data;
}

QVariant ToolButton::data()
{
    return m_data;
}

void ToolButton::setMultiIcon(const QPixmap &image)
{
    int w = image.width();
    int h = image.height();

    m_normalIcon = image.copy(0, 0, w, h / 4);
    m_hoverIcon = image.copy(0, h / 4, w, h / 4);
    m_activeIcon = image.copy(0, h / 2, w, h / 4);
    m_disabledIcon = image.copy(0, 3 * h / 4, w, h / 4);

    m_usingMultiIcon = true;

    setFixedSize(m_normalIcon.size());
}

void ToolButton::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton && menu() && popupMode() == QToolButton::InstantPopup) {
        setDown(true);
        showMenu();
        return;
    }

    if (e->button() == Qt::RightButton && menu()) {
        setDown(true);
        showMenu();
        return;
    }


    if (e->button() == Qt::MiddleButton) {
        setDown(true);
    }

    QToolButton::mousePressEvent(e);
}

void ToolButton::mouseReleaseEvent(QMouseEvent* e)
{
    if (e->button() == Qt::MiddleButton && rect().contains(e->pos())) {
        emit middleMouseClicked();
        setDown(false);
        return;
    }

    if (e->button() == Qt::LeftButton && rect().contains(e->pos()) && e->modifiers() == Qt::ControlModifier) {
        emit controlClicked();
        setDown(false);
        return;
    }

    QToolButton::mouseReleaseEvent(e);
    setDown(false);
}

void ToolButton::showMenu()
{
    if (!m_showMenuInside) {
        QToolButton::showMenu();
        return;
    }

    QMenu* m = menu();

    if (!m) {
        return;
    }

    QPoint pos = mapToGlobal(rect().bottomRight());
    if (QApplication::layoutDirection() == Qt::RightToLeft) {
        pos.setX(pos.x() - rect().width());
    }
    else {
        pos.setX(pos.x() - m->sizeHint().width());
    }

    setDown(true);
    m->exec(pos);
    setDown(false);
}

void ToolButton::paintEvent(QPaintEvent* e)
{
    if (!m_usingMultiIcon) {
        QToolButton::paintEvent(e);
        return;
    }

    QPainter p(this);

    QStyleOptionToolButton opt;
    opt.init(this);

    if (!isEnabled()) {
        p.drawPixmap(0, 0, m_disabledIcon);
        return;
    }

    if (isDown()) {
        p.drawPixmap(0, 0, m_activeIcon);
        return;
    }

    if (opt.state & QStyle::State_MouseOver) {
        p.drawPixmap(0, 0, m_hoverIcon);
        return;
    }

    p.drawPixmap(0, 0, m_normalIcon);
}
