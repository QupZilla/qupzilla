#include "squeezelabelv1.h"

SqueezeLabelV1::SqueezeLabelV1(QWidget* parent)
    : QLabel(parent)
{
}

void SqueezeLabelV1::paintEvent(QPaintEvent* event)
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
