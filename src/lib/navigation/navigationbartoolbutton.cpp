/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2018 David Rosca <nowrep@gmail.com>
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
#include "navigationbartoolbutton.h"
#include "abstractbuttoninterface.h"

#include <QLabel>
#include <QMouseEvent>
#include <QApplication>

NavigationBarToolButton::NavigationBarToolButton(AbstractButtonInterface *button, QWidget *parent)
    : ToolButton(parent)
    , m_button(button)
{
    setAutoRaise(true);
    setToolbarButtonLook(true);
    setFocusPolicy(Qt::NoFocus);

    m_badgeLabel = new QLabel(this);
    m_badgeLabel->setObjectName(QSL("navigation-toolbutton-badge"));
    QFont f = m_badgeLabel->font();
    f.setPixelSize(m_badgeLabel->height() / 2.5);
    m_badgeLabel->setFont(f);
    m_badgeLabel->hide();

    setToolTip(button->toolTip());
    updateIcon();
    updateBadge();

    connect(button, &AbstractButtonInterface::iconChanged, this, &NavigationBarToolButton::updateIcon);
    connect(button, &AbstractButtonInterface::activeChanged, this, &NavigationBarToolButton::updateIcon);
    connect(button, &AbstractButtonInterface::toolTipChanged, this, &NavigationBarToolButton::setToolTip);
    connect(button, &AbstractButtonInterface::badgeTextChanged, this, &NavigationBarToolButton::updateBadge);
    connect(button, &AbstractButtonInterface::visibleChanged, this, &NavigationBarToolButton::visibilityChangeRequested);
}

void NavigationBarToolButton::updateVisibility()
{
    setVisible(m_button->isVisible());
}

void NavigationBarToolButton::clicked()
{
    AbstractButtonInterface::ClickController *c = new AbstractButtonInterface::ClickController;
    c->visualParent = this;
    c->popupPosition = [=](const QSize &size) {
        QPoint pos = mapToGlobal(rect().bottomRight());
        if (QApplication::isRightToLeft()) {
            pos.setX(pos.x() - rect().width());
        } else {
            pos.setX(pos.x() - size.width());
        }
        c->popupOpened = true;
        return pos;
    };
    c->popupClosed = [=]() {
        setDown(false);
        delete c;
    };
    emit m_button->clicked(c);
    if (c->popupOpened) {
        setDown(true);
    } else {
        c->popupClosed();
    }
}

void NavigationBarToolButton::updateIcon()
{
    const QIcon::Mode mode = m_button->isActive() ? QIcon::Normal : QIcon::Disabled;
    const QImage img = m_button->icon().pixmap(iconSize(), mode).toImage();
    setIcon(QPixmap::fromImage(img, Qt::MonoOnly));
}

void NavigationBarToolButton::updateBadge()
{
    if (m_button->badgeText().isEmpty()) {
        m_badgeLabel->hide();
    } else {
        m_badgeLabel->setText(m_button->badgeText());
        m_badgeLabel->resize(m_badgeLabel->sizeHint());
        m_badgeLabel->move(width() - m_badgeLabel->width(), 0);
        m_badgeLabel->show();
    }
}

void NavigationBarToolButton::mouseReleaseEvent(QMouseEvent *e)
{
    // Prevent flickering due to mouse release event restoring Down state

    bool popupOpened = false;

    if (e->button() == Qt::LeftButton && rect().contains(e->pos())) {
        clicked();
        popupOpened = isDown();
    }

    if (popupOpened) {
        setUpdatesEnabled(false);
    }

    ToolButton::mouseReleaseEvent(e);

    if (popupOpened) {
        setDown(true);
        setUpdatesEnabled(true);
    }
}
