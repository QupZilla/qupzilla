#include "lineedit.h"

#include <QEvent>
#include <QLayout>
#include <QStyleOption>
#include <QPainter>
#include <QFocusEvent>

#include <qdebug.h>
SideWidget::SideWidget(QWidget *parent)
    : QWidget(parent)
{
}

bool SideWidget::event(QEvent *event)
{
    if (event->type() == QEvent::LayoutRequest)
        emit sizeHintChanged();
    return QWidget::event(event);
}

LineEdit::LineEdit(QWidget *parent)
    : QLineEdit(parent)
    , m_leftLayout(0)
    , m_rightLayout(0)
    , m_leftMargin(0)
{
    init();
}

LineEdit::LineEdit(const QString &contents, QWidget *parent)
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
    m_leftWidget = new SideWidget(this);
    m_leftWidget->resize(0, 0);
    m_leftLayout = new QHBoxLayout(m_leftWidget);
    m_leftLayout->setContentsMargins(0, 0, 0, 0);

    if (isRightToLeft())
        m_leftLayout->setDirection(QBoxLayout::RightToLeft);
    else
        m_leftLayout->setDirection(QBoxLayout::LeftToRight);
    m_leftLayout->setSizeConstraint(QLayout::SetFixedSize);

    m_rightWidget = new SideWidget(this);
    m_rightWidget->resize(0, 0);
    m_rightLayout = new QHBoxLayout(m_rightWidget);
    if (isRightToLeft())
        m_rightLayout->setDirection(QBoxLayout::RightToLeft);
    else
        m_rightLayout->setDirection(QBoxLayout::LeftToRight);
    m_rightLayout->setContentsMargins(0, 0, 0, 0);

    QSpacerItem *horizontalSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_rightLayout->addItem(horizontalSpacer);

    setWidgetSpacing(3);
    connect(m_leftWidget, SIGNAL(sizeHintChanged()),
            this, SLOT(updateTextMargins()));
    connect(m_rightWidget, SIGNAL(sizeHintChanged()),
            this, SLOT(updateTextMargins()));
}

bool LineEdit::event(QEvent *event)
{
    if (event->type() == QEvent::LayoutDirectionChange) {
        if (isRightToLeft()) {
            m_leftLayout->setDirection(QBoxLayout::RightToLeft);
            m_rightLayout->setDirection(QBoxLayout::RightToLeft);
        } else {
            m_leftLayout->setDirection(QBoxLayout::LeftToRight);
            m_rightLayout->setDirection(QBoxLayout::LeftToRight);
        }
    }
    return QLineEdit::event(event);
}

void LineEdit::addWidget(QWidget *widget, WidgetPosition position)
{
    if (!widget)
        return;

    bool rtl = isRightToLeft();
    if (rtl)
        position = (position == LeftSide) ? RightSide : LeftSide;
    if (position == LeftSide) {
        m_leftLayout->addWidget(widget);
    } else {
        m_rightLayout->insertWidget(1, widget);
    }
}

void LineEdit::removeWidget(QWidget *widget)
{
    if (!widget)
        return;

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
    if (position == LeftSide)
        w = m_leftWidget->sizeHint().width();
    else
        w = m_rightWidget->sizeHint().width();
    if (w == 0)
        return 0;
    return w + spacing * 2;
}

void LineEdit::updateTextMargins()
{
    int left;
    if (m_leftMargin == 0)
        left = textMargin(LineEdit::LeftSide);
    else
        left = m_leftMargin;
    int right = textMargin(LineEdit::RightSide);
    int top = 0;
    int bottom = 0;
    setTextMargins(left, top, right, bottom);
    updateSideWidgetLocations();
}

void LineEdit::updateSideWidgetLocations()
{
    QStyleOptionFrameV2 opt;
    initStyleOption(&opt);
    QRect textRect = style()->subElementRect(QStyle::SE_LineEditContents, &opt, this);
    int spacing = m_rightLayout->spacing();
    textRect.adjust(spacing, 0, -spacing, 0);

    int left = textMargin(LineEdit::LeftSide);

    int midHeight = textRect.center().y() + 1;

    if (m_leftLayout->count() > 0) {
        int leftHeight = midHeight - m_leftWidget->height() / 2;
        int leftWidth = m_leftWidget->width();
        if (leftWidth == 0)
            leftHeight = midHeight - m_leftWidget->sizeHint().height() / 2;
        m_leftWidget->move(textRect.x(), leftHeight);
    }
    textRect.setX(left);
    textRect.setY(midHeight - m_rightWidget->sizeHint().height() / 2);
    textRect.setHeight(m_rightWidget->sizeHint().height());
    m_rightWidget->setGeometry(textRect);
}

void LineEdit::resizeEvent(QResizeEvent *event)
{
    updateSideWidgetLocations();
    QLineEdit::resizeEvent(event);
}

QString LineEdit::inactiveText() const
{
    return m_inactiveText;
}

void LineEdit::setInactiveText(const QString &text)
{
    m_inactiveText = text;
}

void LineEdit::paintEvent(QPaintEvent *event)
{
    QLineEdit::paintEvent(event);
    if (text().isEmpty() && !m_inactiveText.isEmpty() && !hasFocus()) {
        QStyleOptionFrameV2 panel;
        initStyleOption(&panel);
        QRect textRect = style()->subElementRect(QStyle::SE_LineEditContents, &panel, this);
        int horizontalMargin = 2;
        textRect.adjust(horizontalMargin, 0, 0, 0);
        int left = textMargin(LineEdit::LeftSide);
        int right = textMargin(LineEdit::RightSide);
        textRect.adjust(left, 0, -right, 0);
        QPainter painter(this);
        painter.setPen(palette().brush(QPalette::Disabled, QPalette::Text).color());
        painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, m_inactiveText);
    }
}
