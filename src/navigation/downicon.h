#ifndef DOWNICON_H
#define DOWNICON_H

#include "clickablelabel.h"

class DownIcon : public ClickableLabel
{
    Q_OBJECT
public:
    explicit DownIcon(QWidget* parent = 0);

private:
    void mousePressEvent(QMouseEvent *ev);

};

#endif // DOWNICON_H
