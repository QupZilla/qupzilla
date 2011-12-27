#include "rssicon.h"

RssIcon::RssIcon(QWidget *parent)
    : ClickableLabel(parent)
{
    setObjectName("locationbar-rss-icon");
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::ClickFocus);
    setVisible(false);
}

void RssIcon::mousePressEvent(QMouseEvent *ev)
{
    ClickableLabel::mousePressEvent(ev);

    // Prevent propagating to LocationBar
    ev->accept();
}
