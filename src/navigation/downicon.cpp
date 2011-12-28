#include "downicon.h"

DownIcon::DownIcon(QWidget* parent)
    : ClickableLabel(parent)
{
    setObjectName("locationbar-down-icon");
    setCursor(Qt::ArrowCursor);
}

void DownIcon::mousePressEvent(QMouseEvent* ev)
{
    ClickableLabel::mousePressEvent(ev);

    // Prevent propagating to LocationBar
    ev->accept();
}
