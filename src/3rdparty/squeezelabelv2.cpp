#include "squeezelabelv2.h"

SqueezeLabelV2::SqueezeLabelV2(QWidget *parent)
    : QLabel(parent)
{
}

void SqueezeLabelV2::setText(const QString &txt)
{
    m_originalText = txt;
    QFontMetrics fm = fontMetrics();
    QString elided = fm.elidedText(m_originalText, Qt::ElideMiddle, width());
    QLabel::setText(elided);
}

QString SqueezeLabelV2::originalText()
{
    return m_originalText;
}

void SqueezeLabelV2::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);
    QFontMetrics fm = fontMetrics();
    QString elided = fm.elidedText(m_originalText, Qt::ElideMiddle, width());
    QLabel::setText(elided);
}
