#include "progressbar.h"

ProgressBar::ProgressBar(QWidget* parent)
    : QWidget(parent)
    , m_value(0)
    , m_lastPaintedValue(-1)
{
    setMinimumSize(QSize(130,16));
    setMaximumSize(QSize(150,16));
}

void ProgressBar::setValue(int value)
{
    m_value = value;
    if (m_lastPaintedValue != m_value)
        repaint();
}

void ProgressBar::initStyleOption(QStyleOptionProgressBar *option)
{
    if (!option)
        return;

    option->initFrom(this);
    option->minimum = 0;
    option->maximum = 100;
    option->progress = m_value;
    option->textAlignment = Qt::AlignLeft;
    option->textVisible = false;
}

void ProgressBar::paintEvent(QPaintEvent*)
{
    QStylePainter paint(this);
    QStyleOptionProgressBarV2 opt;
    initStyleOption(&opt);
    paint.drawControl(QStyle::CE_ProgressBar, opt);
    m_lastPaintedValue = m_value;
}
