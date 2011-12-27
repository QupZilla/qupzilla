#include "goicon.h"

GoIcon::GoIcon(QWidget *parent)
    : ClickableLabel(parent)
{
    setObjectName("locationbar-goicon");
    setCursor(Qt::PointingHandCursor);
    setHidden(true);
}

void GoIcon::mousePressEvent(QMouseEvent *ev)
{
    ClickableLabel::mousePressEvent(ev);

    // Prevent propagating to LocationBar
    ev->accept();
}
