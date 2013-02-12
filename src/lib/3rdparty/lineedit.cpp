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
#include "lineedit.h"
#include "qzsettings.h"

#include <QEvent>
#include <QLayout>
#include <QStyleOption>
#include <QPainter>
#include <QFocusEvent>

SideWidget::SideWidget(QWidget* parent)
    : QWidget(parent)
{
}

bool SideWidget::event(QEvent* event)
{
    if (event->type() == QEvent::LayoutRequest) {
        emit sizeHintChanged();
    }
    return QWidget::event(event);
}

LineEdit::LineEdit(QWidget* parent)
    : QLineEdit(parent)
    , m_leftLayout(0)
    , m_rightLayout(0)
    , m_leftMargin(-1)
{
    init();
}

LineEdit::LineEdit(const QString &contents, QWidget* parent)
    : QLineEdit(contents, parent)
    , m_leftWidget(0)
    , m_rightWidget(0)
    , m_leftLayout(0)
    , m_rightLayout(0)
    , m_leftMargin(0)
{
    init();
}

void LineEdit::setLeftMargin(int margin)
{
    m_leftMargin = margin;
}

void LineEdit::init()
{
    // We use setTextMargins() instead of padding property, and we should
    // uncomment following line or just update padding property of LineEdit's
    // subclasses in all themes and use same value for padding-left and padding-right,
    // with this new implementation padding-left and padding-right show padding from
    // edges of m_leftWidget and m_rightWidget.

    mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_leftWidget = new SideWidget(this);
    m_leftWidget->resize(0, 0);
    m_leftLayout = new QHBoxLayout(m_leftWidget);
    m_leftLayout->setContentsMargins(0, 0, 2, 0);

    if (isRightToLeft()) {
        m_leftLayout->setDirection(QBoxLayout::RightToLeft);
    }
    else {
        m_leftLayout->setDirection(QBoxLayout::LeftToRight);
    }
    m_leftLayout->setSizeConstraint(QLayout::SetFixedSize);

    m_rightWidget = new SideWidget(this);
    m_rightWidget->resize(0, 0);
    m_rightLayout = new QHBoxLayout(m_rightWidget);
    if (isRightToLeft()) {
        m_rightLayout->setDirection(QBoxLayout::RightToLeft);
    }
    else {
        m_rightLayout->setDirection(QBoxLayout::LeftToRight);
    }

    m_rightLayout->setContentsMargins(0, 0, 2, 0);
    QSpacerItem* horizontalSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

    mainLayout->addWidget(m_leftWidget, 0, Qt::AlignVCenter | Qt::AlignLeft);
    mainLayout->addItem(horizontalSpacer);
    mainLayout->addWidget(m_rightWidget, 0, Qt::AlignVCenter | Qt::AlignRight);
    // by this we undo reversing of layout when direction is RTL.
    // TODO: don't do this and show reversed icon when needed
    mainLayout->setDirection(isRightToLeft() ? QBoxLayout::RightToLeft : QBoxLayout::LeftToRight);

    setWidgetSpacing(3);
    connect(m_leftWidget, SIGNAL(sizeHintChanged()),
            this, SLOT(updateTextMargins()));
    connect(m_rightWidget, SIGNAL(sizeHintChanged()),
            this, SLOT(updateTextMargins()));
}

bool LineEdit::event(QEvent* event)
{
    if (event->type() == QEvent::LayoutDirectionChange) {
        //by this we undo reversing of layout when direction is RTL.
        if (isRightToLeft()) {
            mainLayout->setDirection(QBoxLayout::RightToLeft);
            m_leftLayout->setDirection(QBoxLayout::RightToLeft);
            m_rightLayout->setDirection(QBoxLayout::RightToLeft);
        }
        else {
            mainLayout->setDirection(QBoxLayout::LeftToRight);
            m_leftLayout->setDirection(QBoxLayout::LeftToRight);
            m_rightLayout->setDirection(QBoxLayout::LeftToRight);
        }
    }
    return QLineEdit::event(event);
}

void LineEdit::addWidget(QWidget* widget, WidgetPosition position)
{
    if (!widget) {
        return;
    }
    if (position == LeftSide) {
        m_leftLayout->addWidget(widget);
    }
    else {
        m_rightLayout->addWidget(widget);
    }
}

void LineEdit::removeWidget(QWidget* widget)
{
    if (!widget) {
        return;
    }

    m_leftLayout->removeWidget(widget);
    m_rightLayout->removeWidget(widget);
    widget->hide();
}

void LineEdit::setWidgetSpacing(int spacing)
{
    m_leftLayout->setSpacing(spacing);
    m_rightLayout->setSpacing(spacing);
    updateTextMargins();
}

int LineEdit::widgetSpacing() const
{
    return m_leftLayout->spacing();
}

int LineEdit::textMargin(WidgetPosition position) const
{
    int spacing = m_rightLayout->spacing();
    int w = 0;
    if (position == LeftSide) {
        w = m_leftWidget->sizeHint().width();
    }
    else {
        w = m_rightWidget->sizeHint().width();
    }
    if (w == 0) {
        return 0;
    }
    return w + spacing * 2;
}

void LineEdit::updateTextMargins()
{
    int left = m_leftMargin < 0 ? m_leftWidget->sizeHint().width() : m_leftMargin;
    int right = m_rightWidget->sizeHint().width();
    int top = 0;
    int bottom = 0;
    setTextMargins(left, top, right, bottom);
    //    updateSideWidgetLocations();
}

void LineEdit::mousePressEvent(QMouseEvent* event)
{
    if (cursorPosition() == 0 && qzSettings->selectAllOnClick) {
        selectAll();
        return;
    }

    QLineEdit::mousePressEvent(event);
}

void LineEdit::mouseReleaseEvent(QMouseEvent* event)
{
    // Workaround issue in QLineEdit::setDragEnabled(true)
    // It will incorrectly set cursor position at the end
    // of selection when clicking (and not dragging) into selected text

    if (!dragEnabled()) {
        QLineEdit::mouseReleaseEvent(event);
        return;
    }

    bool wasSelectedText = !selectedText().isEmpty();

    QLineEdit::mouseReleaseEvent(event);

    bool isSelectedText = !selectedText().isEmpty();

    if (wasSelectedText && !isSelectedText) {
        QMouseEvent ev(QEvent::MouseButtonPress, event->pos(), event->button(),
                       event->buttons(), event->modifiers());
        mousePressEvent(&ev);
    }
}

void LineEdit::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && qzSettings->selectAllOnDoubleClick) {
        selectAll();
        return;
    }

    QLineEdit::mouseDoubleClickEvent(event);
}
