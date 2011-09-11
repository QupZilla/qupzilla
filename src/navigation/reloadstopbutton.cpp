#include "reloadstopbutton.h"

ReloadStopButton::ReloadStopButton(QWidget *parent) :
    QWidget(parent)
{
    QHBoxLayout* lay = new QHBoxLayout(this);
    setLayout(lay);

    m_buttonStop = new ToolButton();
    m_buttonStop->setObjectName("navigation-button-stop");
    m_buttonStop->setToolTip(tr("Stop"));
    m_buttonStop->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_buttonStop->setVisible(false);
    m_buttonStop->setAutoRaise(true);

    m_buttonReload = new ToolButton();
    m_buttonReload->setObjectName("navigation-button-reload");
    m_buttonReload->setToolTip(tr("Reload"));
    m_buttonReload->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_buttonReload->setAutoRaise(true);

    lay->addWidget(m_buttonStop);
    lay->addWidget(m_buttonReload);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);
}

void ReloadStopButton::showReloadButton()
{
    setUpdatesEnabled(false);
    m_buttonStop->hide();
    m_buttonReload->show();
    setUpdatesEnabled(true);
}

void ReloadStopButton::showStopButton()
{
    setUpdatesEnabled(false);
    m_buttonReload->hide();
    m_buttonStop->show();
    setUpdatesEnabled(true);
}

ReloadStopButton::~ReloadStopButton()
{
    delete m_buttonStop;
    delete m_buttonReload;
}
