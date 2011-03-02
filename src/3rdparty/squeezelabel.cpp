#include "squeezelabel.h"

SqueezeLabel::SqueezeLabel(QWidget *parent)
    : QLabel(parent)
{
}

void SqueezeLabel::paintEvent(QPaintEvent *event)
{
    if (m_SqueezedTextCache != text()) {
        m_SqueezedTextCache = text();
        QFontMetrics fm = fontMetrics();
        if (fm.width(m_SqueezedTextCache) > contentsRect().width()) {
            QString elided = fm.elidedText(text(), Qt::ElideMiddle, width());
            setText(elided);
        }
    }
    QLabel::paintEvent(event);
}
