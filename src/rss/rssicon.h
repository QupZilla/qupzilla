#ifndef RSSICON_H
#define RSSICON_H

#include "clickablelabel.h"

class RssIcon : public ClickableLabel
{
    Q_OBJECT
public:
    explicit RssIcon(QWidget *parent = 0);

signals:

public slots:

private:
    void mousePressEvent(QMouseEvent *ev);
};

#endif // RSSICON_H
