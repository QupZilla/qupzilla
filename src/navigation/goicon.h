#ifndef GOICON_H
#define GOICON_H

#include "clickablelabel.h"

class GoIcon : public ClickableLabel
{
public:
    explicit GoIcon(QWidget* parent = 0);

private:
    void mousePressEvent(QMouseEvent* ev);

};

#endif // GOICON_H
