/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2017 David Rosca <nowrep@gmail.com>
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

SideWidget::SideWidget(QWidget* parent)
    : QWidget(parent)
{
    setCursor(Qt::ArrowCursor);
    setFocusPolicy(Qt::ClickFocus);
}

bool SideWidget::event(QEvent* event)
{
    switch (event->type()) {
    case QEvent::LayoutRequest:
        emit sizeHintChanged();
        break;

    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
        event->accept();
        return true;

    default:
        break;
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

    QAction* undoAction = new QAction(QIcon::fromTheme(QSL("edit-undo")), tr("&Undo"), this);
    undoAction->setShortcut(QKeySequence(QSL("Ctrl+Z")));
    undoAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(undoAction, SIGNAL(triggered()), SLOT(undo()));

    QAction* redoAction = new QAction(QIcon::fromTheme(QSL("edit-redo")), tr("&Redo"), this);
    redoAction->setShortcut(QKeySequence(QSL("Ctrl+Shift+Z")));
    redoAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(redoAction, SIGNAL(triggered()), SLOT(redo()));

    QAction* cutAction = new QAction(QIcon::fromTheme(QSL("edit-cut")), tr("Cu&t"), this);
    cutAction->setShortcut(QKeySequence(QSL("Ctrl+X")));
    cutAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(cutAction, SIGNAL(triggered()), SLOT(cut()));

    QAction* copyAction = new QAction(QIcon::fromTheme(QSL("edit-copy")), tr("&Copy"), this);
    copyAction->setShortcut(QKeySequence(QSL("Ctrl+C")));
    copyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(copyAction, SIGNAL(triggered()), SLOT(copy()));

    QAction* pasteAction = new QAction(QIcon::fromTheme(QSL("edit-paste")), tr("&Paste"), this);
    pasteAction->setShortcut(QKeySequence(QSL("Ctrl+V")));
    pasteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(pasteAction, SIGNAL(triggered()), SLOT(paste()));

    QAction* pasteAndGoAction = new QAction(this);
    pasteAndGoAction->setShortcut(QKeySequence(QSL("Ctrl+Shift+V")));
    pasteAndGoAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);

    QAction* deleteAction = new QAction(QIcon::fromTheme(QSL("edit-delete")), tr("Delete"), this);
    connect(deleteAction, SIGNAL(triggered()), SLOT(slotDelete()));

    QAction* clearAllAction = new QAction(QIcon::fromTheme(QSL("edit-clear")), tr("Clear All"), this);
    connect(clearAllAction, SIGNAL(triggered()), SLOT(clear()));

    QAction* selectAllAction = new QAction(QIcon::fromTheme(QSL("edit-select-all")), tr("Select All"), this);
    selectAllAction->setShortcut(QKeySequence(QSL("Ctrl+A")));
    selectAllAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(selectAllAction, SIGNAL(triggered()), SLOT(selectAll()));

    m_editActions[Undo] = undoAction;
    m_editActions[Redo] = redoAction;
    m_editActions[Cut] = cutAction;
    m_editActions[Copy] = copyAction;
    m_editActions[Paste] = pasteAction;
    m_editActions[PasteAndGo] = pasteAndGoAction;
    m_editActions[Delete] = deleteAction;
    m_editActions[ClearAll] = clearAllAction;
    m_editActions[SelectAll] = selectAllAction;

    // Make action shortcuts available for webview
    addAction(undoAction);
    addAction(redoAction);
    addAction(cutAction);
    addAction(copyAction);
    addAction(pasteAction);
    addAction(pasteAndGoAction);
    addAction(deleteAction);
    addAction(clearAllAction);
    addAction(selectAllAction);

    // Connections to update edit actions
    connect(this, SIGNAL(textChanged(QString)), this, SLOT(updateActions()));
    connect(this, SIGNAL(selectionChanged()), this, SLOT(updateActions()));

    updateActions();
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
QMenu* LineEdit::createContextMenu()
{
    QMenu* popup = new QMenu(this);
    popup->setObjectName(QSL("qt_edit_menu"));

    if (!isReadOnly()) {
        popup->addAction(m_editActions[Undo]);
        popup->addAction(m_editActions[Redo]);
        popup->addSeparator();
        popup->addAction(m_editActions[Cut]);
    }

    popup->addAction(m_editActions[Copy]);

    if (!isReadOnly()) {
        updatePasteActions();
        popup->addAction(m_editActions[Paste]);
        if (!m_editActions[PasteAndGo]->text().isEmpty()) {
            popup->addAction(m_editActions[PasteAndGo]);
        }
        popup->addAction(m_editActions[Delete]);
        popup->addAction(m_editActions[ClearAll]);
    }

    popup->addSeparator();
    popup->addAction(m_editActions[SelectAll]);

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

void LineEdit::updateActions()
{
    m_editActions[Undo]->setEnabled(!isReadOnly() && isUndoAvailable());
    m_editActions[Redo]->setEnabled(!isReadOnly() && isRedoAvailable());
    m_editActions[Cut]->setEnabled(!isReadOnly() && hasSelectedText() && echoMode() == QLineEdit::Normal);
    m_editActions[Copy]->setEnabled(hasSelectedText() && echoMode() == QLineEdit::Normal);
    m_editActions[Delete]->setEnabled(!isReadOnly() && hasSelectedText());
    m_editActions[SelectAll]->setEnabled(!text().isEmpty() && selectedText() != text());
    m_editActions[Paste]->setEnabled(true);
    m_editActions[PasteAndGo]->setEnabled(true);
}

void LineEdit::updatePasteActions()
{
    // Paste actions are updated in separate slot because accessing clipboard is expensive
    bool pasteEnabled = !isReadOnly() && !QApplication::clipboard()->text().isEmpty();

    m_editActions[Paste]->setEnabled(pasteEnabled);
    m_editActions[PasteAndGo]->setEnabled(pasteEnabled);
}

void LineEdit::slotDelete()
{
    if (hasSelectedText()) {
        del();
    }
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

QAction* LineEdit::editAction(EditAction action) const
{
    return m_editActions[action];
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

void LineEdit::resizeEvent(QResizeEvent *event)
{
    QLineEdit::resizeEvent(event);

    m_leftWidget->setFixedHeight(height());
    m_rightWidget->setFixedHeight(height());
}
