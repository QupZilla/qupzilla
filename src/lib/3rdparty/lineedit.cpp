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
#include "lineedit.h"
#include "qzsettings.h"

#include <QMenu>
#include <QEvent>
#include <QLayout>
#include <QPainter>
#include <QClipboard>
#include <QFocusEvent>
#include <QStyleOption>
#include <QApplication>

#if QT_VERSION < 0x050000
#include <QInputContext>
#endif

SideWidget::SideWidget(QWidget* parent)
    : QWidget(parent)
{
    setCursor(Qt::ArrowCursor);
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
    , m_minHeight(0)
    , m_leftMargin(-1)
    , m_ignoreMousePress(false)
{
    init();
}

void LineEdit::setLeftMargin(int margin)
{
    m_leftMargin = margin;
}

void LineEdit::init()
{
    mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_leftWidget = new SideWidget(this);
    m_leftWidget->resize(0, 0);
    m_leftLayout = new QHBoxLayout(m_leftWidget);
    m_leftLayout->setContentsMargins(0, 0, 0, 0);
    m_leftLayout->setDirection(isRightToLeft() ? QBoxLayout::RightToLeft : QBoxLayout::LeftToRight);

    m_rightWidget = new SideWidget(this);
    m_rightWidget->resize(0, 0);
    m_rightLayout = new QHBoxLayout(m_rightWidget);
    m_rightLayout->setDirection(isRightToLeft() ? QBoxLayout::RightToLeft : QBoxLayout::LeftToRight);
    m_rightLayout->setContentsMargins(0, 0, 2, 0);

    mainLayout->addWidget(m_leftWidget, 0, Qt::AlignVCenter | Qt::AlignLeft);
    mainLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
    mainLayout->addWidget(m_rightWidget, 0, Qt::AlignVCenter | Qt::AlignRight);
    mainLayout->setDirection(isRightToLeft() ? QBoxLayout::RightToLeft : QBoxLayout::LeftToRight);

    setWidgetSpacing(3);

    connect(m_leftWidget, SIGNAL(sizeHintChanged()), this, SLOT(updateTextMargins()));
    connect(m_rightWidget, SIGNAL(sizeHintChanged()), this, SLOT(updateTextMargins()));
}

bool LineEdit::event(QEvent* event)
{
    if (event->type() == QEvent::LayoutDirectionChange) {
        // By this we undo reversing of layout when direction is RTL.
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

#define ACCEL_KEY(k) QLatin1Char('\t') + QKeySequence(k).toString()

// Modified QLineEdit::createStandardContextMenu to support icons and PasteAndGo action
QMenu* LineEdit::createContextMenu(QAction* pasteAndGoAction)
{
    QMenu* popup = new QMenu(this);
    popup->setObjectName(QSL("qt_edit_menu"));

    QAction* action = 0;

    if (!isReadOnly()) {
        action = popup->addAction(QIcon::fromTheme(QSL("edit-undo")), tr("&Undo") + ACCEL_KEY(QKeySequence::Undo));
        action->setEnabled(isUndoAvailable());
        connect(action, SIGNAL(triggered()), SLOT(undo()));

        action = popup->addAction(QIcon::fromTheme(QSL("edit-redo")), tr("&Redo") + ACCEL_KEY(QKeySequence::Redo));
        action->setEnabled(isRedoAvailable());
        connect(action, SIGNAL(triggered()), SLOT(redo()));

        popup->addSeparator();
    }

#ifndef QT_NO_CLIPBOARD
    if (!isReadOnly()) {
        action = popup->addAction(QIcon::fromTheme(QSL("edit-cut")), tr("Cu&t") + ACCEL_KEY(QKeySequence::Cut));
        action->setEnabled(hasSelectedText() && echoMode() == QLineEdit::Normal);
        connect(action, SIGNAL(triggered()), SLOT(cut()));
    }

    action = popup->addAction(QIcon::fromTheme(QSL("edit-copy")), tr("&Copy") + ACCEL_KEY(QKeySequence::Copy));
    action->setEnabled(hasSelectedText() && echoMode() == QLineEdit::Normal);
    connect(action, SIGNAL(triggered()), SLOT(copy()));

    if (!isReadOnly()) {
        action = popup->addAction(QIcon::fromTheme(QSL("edit-paste")), tr("&Paste") + ACCEL_KEY(QKeySequence::Paste));
        action->setEnabled(!QApplication::clipboard()->text().isEmpty());
        connect(action, SIGNAL(triggered()), SLOT(paste()));

        pasteAndGoAction->setEnabled(action->isEnabled());
        popup->addAction(pasteAndGoAction);
    }
#endif

    if (!isReadOnly()) {
        action = popup->addAction(QIcon::fromTheme(QSL("edit-delete")), tr("Delete") + ACCEL_KEY(QKeySequence::Delete));
        action->setEnabled(hasSelectedText());
        connect(action, SIGNAL(triggered()), this, SLOT(slotDelete()));

        action = popup->addAction(QIcon::fromTheme(QSL("edit-clear")), tr("Clear All"));
        connect(action, SIGNAL(triggered()), this, SLOT(clear()));
    }

    if (!popup->isEmpty()) {
        popup->addSeparator();
    }

    action = popup->addAction(QIcon::fromTheme(QSL("edit-select-all")), tr("Select All") + ACCEL_KEY(QKeySequence::SelectAll));
    action->setEnabled(!text().isEmpty() && selectedText() != text());
    connect(action, SIGNAL(triggered()), SLOT(selectAll()));

#if !defined(QT_NO_IM) && QT_VERSION < 0x050000
    QInputContext* qic = inputContext();
    if (qic) {
        QList<QAction*> imActions = qic->actions();
        for (int i = 0; i < imActions.size(); ++i) {
            popup->addAction(imActions.at(i));
        }
    }
#endif

    // Hack to get QUnicodeControlCharacterMenu
    QMenu* tmp = createStandardContextMenu();
    tmp->setParent(popup);
    tmp->hide();
    QAction* lastAction = !tmp->actions().isEmpty() ? tmp->actions().last() : 0;

    if (lastAction && lastAction->menu() && lastAction->menu()->inherits("QUnicodeControlCharacterMenu")) {
        popup->addAction(lastAction);
    }

    return popup;
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

int LineEdit::leftMargin() const
{
    return m_leftMargin;
}

// http://stackoverflow.com/a/14424003
void LineEdit::setTextFormat(const LineEdit::TextFormat &format)
{
    QList<QInputMethodEvent::Attribute> attributes;

    foreach (const QTextLayout::FormatRange &fr, format) {
        QInputMethodEvent::AttributeType type = QInputMethodEvent::TextFormat;
        int start = fr.start - cursorPosition();
        int length = fr.length;
        QVariant value = fr.format;
        attributes.append(QInputMethodEvent::Attribute(type, start, length, value));
    }

    QInputMethodEvent ev(QString(), attributes);
    event(&ev);
}

void LineEdit::clearTextFormat()
{
    setTextFormat(TextFormat());
}

int LineEdit::minHeight() const
{
    return m_minHeight;
}

void LineEdit::setMinHeight(int height)
{
    m_minHeight = height;
}

QSize LineEdit::sizeHint() const
{
    QSize s = QLineEdit::sizeHint();

    if (s.height() < m_minHeight) {
        s.setHeight(m_minHeight);
    }

    return s;
}

void LineEdit::updateTextMargins()
{
    int left = m_leftWidget->sizeHint().width();
    int right = m_rightWidget->sizeHint().width();
    int top = 0;
    int bottom = 0;

    if (m_leftMargin >= 0) {
        left = m_leftMargin;
    }

    setTextMargins(left, top, right, bottom);
}

void LineEdit::slotDelete()
{
    if (hasSelectedText()) {
        del();
    }
}

void LineEdit::focusInEvent(QFocusEvent* event)
{
    if (event->reason() == Qt::MouseFocusReason && qzSettings->selectAllOnClick) {
        m_ignoreMousePress = true;
        selectAll();
    }

    QLineEdit::focusInEvent(event);
}

void LineEdit::mousePressEvent(QMouseEvent* event)
{
    if (m_ignoreMousePress) {
        m_ignoreMousePress = false;
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
    if (event->buttons() == Qt::LeftButton && qzSettings->selectAllOnDoubleClick) {
        selectAll();
        return;
    }

    QLineEdit::mouseDoubleClickEvent(event);
}
