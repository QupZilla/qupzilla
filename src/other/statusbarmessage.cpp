#include "statusbarmessage.h"
#include "qupzilla.h"
#include "squeezelabelv1.h"

TipLabel::TipLabel(QupZilla* parent) : SqueezeLabelV1(parent) , p_QupZilla(parent)
{
    m_timer = new QTimer();
    m_timer->setInterval(300);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(checkMainWindowFocus()));

    setWindowFlags(Qt::ToolTip);
    setForegroundRole(QPalette::ToolTipText);
    setBackgroundRole(QPalette::ToolTipBase);
    setPalette(QToolTip::palette());
    ensurePolished();
    setFrameStyle(QFrame::NoFrame);
    setMargin(2);
}

void TipLabel::paintEvent(QPaintEvent *ev)
{
    QStylePainter p(this);
    QStyleOptionFrame opt;
    opt.init(this);
    p.drawPrimitive(QStyle::PE_PanelTipLabel, opt);
    p.end();

    SqueezeLabelV1::paintEvent(ev);
}

void TipLabel::show()
{
    if (!p_QupZilla->weView()->hasFocus())
        return;

    m_timer->start();
    SqueezeLabelV1::show();
}

void TipLabel::hide()
{
    m_timer->stop();
    SqueezeLabelV1::hide();
}

void TipLabel::checkMainWindowFocus()
{
    if (!p_QupZilla->weView()->hasFocus())
        hide();
}



StatusBarMessage::StatusBarMessage(QupZilla* mainClass)
    : QObject()
    , p_QupZilla(mainClass)
{
    m_statusBarText = new TipLabel(mainClass);
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
