#include "squeezelabel.h"

SqueezeLabel::SqueezeLabel(QWidget *parent)
    : QLabel(parent)
{
}

void SqueezeLabel::setText(const QString &txt)
{
    m_originalText = txt;
    QFontMetrics fm = fontMetrics();
    QString elided = fm.elidedText(m_originalText, Qt::ElideMiddle, width());
    QLabel::setText(elided);
}

QString SqueezeLabel::originalText()
{
    return m_originalText;
}

void SqueezeLabel::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);
    QFontMetrics fm = fontMetrics();
    QString elided = fm.elidedText(m_originalText, Qt::ElideMiddle, width());
    QLabel::setText(elided);
}
