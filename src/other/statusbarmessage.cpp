#include "statusbarmessage.h"
#include "qupzilla.h"
#include "squeezelabel.h"


class TipLabel : public SqueezeLabel {
public:
    TipLabel()
    {
        setWindowFlags(Qt::ToolTip);
        setForegroundRole(QPalette::ToolTipText);
        setBackgroundRole(QPalette::ToolTipBase);
        setPalette(QToolTip::palette());
        ensurePolished();
        setFrameStyle(QFrame::NoFrame);
        setMargin(2);
    }

    void paintEvent(QPaintEvent *ev)
    {
        QStylePainter p(this);
        QStyleOptionFrame opt;
        opt.init(this);
        p.drawPrimitive(QStyle::PE_PanelTipLabel, opt);
        p.end();

        SqueezeLabel::paintEvent(ev);
    }
};


StatusBarMessage::StatusBarMessage(QupZilla* mainClass)
    : QObject()
    , p_QupZilla(mainClass)
{
    m_statusBarText = new TipLabel();
}

#define STATUS_Y_OFFSET -35
#define STATUS_X_OFFSET +3
#define STATUS_TOOLTIP_WIDTH_OFFSET -20

void StatusBarMessage::showMessage(const QString &message)
{
    if (p_QupZilla->statusBar()->isVisible()) {
        p_QupZilla->statusBar()->showMessage(message);
    } else {
        QPoint position = p_QupZilla->weView()->mapToGlobal(p_QupZilla->weView()->pos());
        position.setY( p_QupZilla->weView()->size().height() + position.y() STATUS_Y_OFFSET);
        position.setX(position.x() STATUS_X_OFFSET);
        m_statusBarText->move(position);
        m_statusBarText->setMaximumWidth(p_QupZilla->size().width() STATUS_TOOLTIP_WIDTH_OFFSET);
        m_statusBarText->setText(message);
        m_statusBarText->resize(m_statusBarText->sizeHint());
        m_statusBarText->show();
    }
}

void StatusBarMessage::clearMessage()
{
    if (p_QupZilla->statusBar()->isVisible())
        p_QupZilla->statusBar()->showMessage("");
    else
        m_statusBarText->hide();
}
