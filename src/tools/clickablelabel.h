#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>
#include <QMouseEvent>

class ClickableLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ClickableLabel(QWidget *parent = 0);

signals:
    void clicked(QPoint);

private:
    void mousePressEvent(QMouseEvent *ev);
    void mouseDoubleClickEvent(QMouseEvent *ev);

};

#endif // CLICKABLELABEL_H
