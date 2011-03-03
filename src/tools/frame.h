#ifndef FRAME_H
#define FRAME_H

#include <QFrame>
#include <QMouseEvent>

class Frame : public QFrame
{
    Q_OBJECT
public:
    explicit Frame(QWidget *parent = 0);

signals:

public slots:

private:
    void mousePressEvent(QMouseEvent *event);

};

#endif // FRAME_H
