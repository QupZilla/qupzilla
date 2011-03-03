#include "frame.h"

Frame::Frame(QWidget *parent) :
    QFrame(parent)
{
}

void Frame::mousePressEvent(QMouseEvent *event)
{
    //If we proccess mouse events, then menu from bookmarkswidget
    //is going to close() with clicking in free space
    Q_UNUSED(event)
    event->accept();
}
