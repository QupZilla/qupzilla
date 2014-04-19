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
{
    setMinimumWidth(16);
}

QPixmap ToolButton::pixmap() const
{
    return m_normalIcon;
}

void ToolButton::setMultiIcon(const QPixmap &icon)
{
    int w = icon.width();
    int h = icon.height();

    m_normalIcon = icon.copy(0, 0, w, h / 4);
    m_hoverIcon = icon.copy(0, h / 4, w, h / 4);
    m_activeIcon = icon.copy(0, h / 2, w, h / 4);
    m_disabledIcon = icon.copy(0, 3 * h / 4, w, h / 4);

    m_usingMultiIcon = true;
    setFixedSize(m_normalIcon.size());
}

QString ToolButton::themeIcon() const
{
    return m_themeIcon;
}

void ToolButton::setThemeIcon(const QString &icon)
{
    m_themeIcon = icon;
    setIcon(QIcon::fromTheme(m_themeIcon));
}

void ToolButton::setIcon(const QIcon &icon)
{
    if (m_usingMultiIcon)
        setFixedSize(sizeHint());

    m_usingMultiIcon = false;
    QToolButton::setIcon(icon);
}

void ToolButton::setMenu(QMenu* menu)
{
    Q_ASSERT(menu);

    if (QToolButton::menu())
        disconnect(QToolButton::menu(), SIGNAL(aboutToHide()), this, SLOT(menuAboutToHide()));

    connect(menu, SIGNAL(aboutToHide()), this, SLOT(menuAboutToHide()));
    QToolButton::setMenu(menu);
}

bool ToolButton::showMenuInside() const
{
    return m_showMenuInside;
}

void ToolButton::setShowMenuInside(bool inside)
{
    m_showMenuInside = inside;
}

void ToolButton::menuAboutToHide()
{
    setDown(false);
}

void ToolButton::mousePressEvent(QMouseEvent* e)
{
    if (e->buttons() == Qt::LeftButton && menu() && popupMode() == QToolButton::InstantPopup) {
        setDown(true);
        showMenu();
        return;
    }

    if (e->buttons() == Qt::RightButton && menu()) {
        setDown(true);
        showMenu();
        return;
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
}

void ToolButton::mouseDoubleClickEvent(QMouseEvent* e)
{
    QToolButton::mouseDoubleClickEvent(e);

    if (e->buttons() == Qt::LeftButton) {
        emit doubleClicked();
    }
}

void ToolButton::paintEvent(QPaintEvent* e)
{
    if (!m_usingMultiIcon) {
        QToolButton::paintEvent(e);
        return;
    }

    QPainter p(this);

    if (!isEnabled())
        p.drawPixmap(0, 0, m_disabledIcon);
    else if (isDown())
        p.drawPixmap(0, 0, m_activeIcon);
    else if (underMouse())
        p.drawPixmap(0, 0, m_hoverIcon);
    else
        p.drawPixmap(0, 0, m_normalIcon);
}

void ToolButton::showMenu()
{
    if (!m_showMenuInside) {
        QToolButton::showMenu();
        return;
    }

    QMenu* m = menu();
    if (!m)
        return;

    emit aboutToShowMenu();

    QPoint pos = mapToGlobal(rect().bottomRight());
    if (QApplication::layoutDirection() == Qt::RightToLeft)
        pos.setX(pos.x() - rect().width());
    else
        pos.setX(pos.x() - m->sizeHint().width());
    m->popup(pos);
}
