#include "widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent)
{
}

void Widget::slotResize(const QSize &size)
{
    resize(size);
}
