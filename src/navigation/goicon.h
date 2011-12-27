#ifndef GOICON_H
#define GOICON_H

#include "clickablelabel.h"

class GoIcon : public ClickableLabel
{
    Q_OBJECT
public:
    explicit GoIcon(QWidget *parent = 0);

signals:

public slots:

private:
    void mousePressEvent(QMouseEvent *ev);

};

#endif // GOICON_H
