
#include <QLayout>

#include "locationbarpopup.h"

LocationBarPopup::LocationBarPopup(QWidget* parent)
    : QFrame(parent, Qt::Popup)
    , m_alignment(Qt::AlignRight)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFrameStyle(QFrame::Panel | QFrame::Raised);
    setLineWidth(1);
    setMidLineWidth(2);
}

void LocationBarPopup::showAt(QWidget* parent)
{
    layout()->invalidate();
    layout()->activate();

    QPoint p = parent->mapToGlobal(QPoint(0, 0));
    if (m_alignment == Qt::AlignRight) {
        p.setX(p.x() + parent->width() - width());
    }
    p.setY(p.y() + parent->height());
    move(p);

    show();
}
