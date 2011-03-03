#ifndef BOOKMARKSTREEWIDGET_H
#define BOOKMARKSTREEWIDGET_H

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QMouseEvent>

class TreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit TreeWidget(QWidget *parent = 0);

signals:
    void itemControlClicked(QTreeWidgetItem *item);

public slots:

private:
    void mousePressEvent(QMouseEvent *event);

};

#endif // BOOKMARKSTREEWIDGET_H
