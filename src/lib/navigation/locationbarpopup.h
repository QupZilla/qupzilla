
#ifndef QUPZILLA_LOCATION_BAR_POPUP
#define QUPZILLA_LOCATION_BAR_POPUP

#include <QFrame>

class LocationBarPopup : public QFrame
{
public:
    LocationBarPopup(QWidget* parent);
    void showAt(QWidget* parent);
    void setPopupAlignment(Qt::Alignment alignment) {
        m_alignment = alignment;
    }

    Qt::Alignment popupAlignment() const {
        return m_alignment;
    }

private:
    Qt::Alignment m_alignment;
};

#endif
