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

#include <QMenu>
#include <QStyle>
#include <QPainter>
#include <QMouseEvent>
#include <QApplication>
#include <QStyleOptionToolButton>

ToolButton::ToolButton(QWidget* parent)
    : QToolButton(parent)
    , m_menu(0)
{
    setMinimumWidth(16);

    QStyleOptionToolButton opt;
    initStyleOption(&opt);

    m_pressTimer.setSingleShot(true);
    m_pressTimer.setInterval(QApplication::style()->styleHint(QStyle::SH_ToolButton_PopupDelay, &opt, this));
    connect(&m_pressTimer, SIGNAL(timeout()), this, SLOT(showMenu()));
}

QPixmap ToolButton::multiIcon() const
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

    m_options |= MultiIconOption;
    setFixedSize(m_normalIcon.size());

    update();
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

QIcon ToolButton::icon() const
{
    return m_options & MultiIconOption ? multiIcon() : QToolButton::icon();
}

void ToolButton::setIcon(const QIcon &icon)
{
    if (m_options & MultiIconOption)
        setFixedSize(sizeHint());

    m_options &= ~MultiIconOption;
    QToolButton::setIcon(icon);
}

QMenu* ToolButton::menu() const
{
    return m_menu;
}

void ToolButton::setMenu(QMenu* menu)
{
    Q_ASSERT(menu);

    if (m_menu)
        disconnect(m_menu, SIGNAL(aboutToHide()), this, SLOT(menuAboutToHide()));

    m_menu = menu;
    connect(m_menu, SIGNAL(aboutToHide()), this, SLOT(menuAboutToHide()));
}

bool ToolButton::showMenuInside() const
{
    return m_options & ShowMenuInsideOption;
}

void ToolButton::setShowMenuInside(bool enable)
{
    if (enable)
        m_options |= ShowMenuInsideOption;
    else
        m_options &= ~ShowMenuInsideOption;
}

bool ToolButton::toolbarButtonLook() const
{
    return m_options & ToolBarLookOption;
}

void ToolButton::setToolbarButtonLook(bool enable)
{
    if (enable) {
        m_options |= ToolBarLookOption;

        QStyleOption opt;
        opt.initFrom(this);
        int size = style()->pixelMetric(QStyle::PM_ToolBarIconSize, &opt, this);
        setIconSize(QSize(size, size));
    }
    else {
        m_options &= ~ToolBarLookOption;
    }

    setProperty("toolbar-look", QVariant(enable));
    style()->unpolish(this);
    style()->polish(this);
}

void ToolButton::menuAboutToHide()
{
    setDown(false);
    emit aboutToHideMenu();
}

void ToolButton::showMenu()
{
    if (!m_menu || m_menu->isVisible())
        return;

    emit aboutToShowMenu();

    QPoint pos;

    if (m_options & ShowMenuInsideOption) {
        pos = mapToGlobal(rect().bottomRight());
        if (QApplication::layoutDirection() == Qt::RightToLeft)
            pos.setX(pos.x() - rect().width());
        else
            pos.setX(pos.x() - m_menu->sizeHint().width());
    }
    else {
        pos = mapToGlobal(rect().bottomLeft());
    }

    m_menu->popup(pos);
}

void ToolButton::mousePressEvent(QMouseEvent* e)
{
    QToolButton::mousePressEvent(e);

    if (popupMode() == QToolButton::DelayedPopup)
        m_pressTimer.start();

    if (e->buttons() == Qt::LeftButton && menu() && popupMode() == QToolButton::InstantPopup) {
        setDown(true);
        showMenu();
    }
    else if (e->buttons() == Qt::RightButton && menu()) {
        setDown(true);
        showMenu();
    }
}

void ToolButton::mouseReleaseEvent(QMouseEvent* e)
{
    QToolButton::mouseReleaseEvent(e);

    m_pressTimer.stop();

    if (e->button() == Qt::MiddleButton && rect().contains(e->pos())) {
        emit middleMouseClicked();
        setDown(false);
    }
    else if (e->button() == Qt::LeftButton && rect().contains(e->pos()) && e->modifiers() == Qt::ControlModifier) {
        emit controlClicked();
        setDown(false);
    }
}

void ToolButton::mouseDoubleClickEvent(QMouseEvent* e)
{
    QToolButton::mouseDoubleClickEvent(e);

    m_pressTimer.stop();

    if (e->buttons() == Qt::LeftButton) {
        emit doubleClicked();
    }
}

void ToolButton::paintEvent(QPaintEvent* e)
{
    if (!(m_options & MultiIconOption)) {
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
