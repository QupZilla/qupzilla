#include "treewidget.h"

TreeWidget::TreeWidget(QWidget *parent) :
    QTreeWidget(parent)
{
}

void TreeWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->modifiers() == Qt::ControlModifier)
        emit itemControlClicked(itemAt(event->pos()));

    QTreeWidget::mousePressEvent(event);
}
