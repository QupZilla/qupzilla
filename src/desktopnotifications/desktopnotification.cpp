#include "desktopnotification.h"
#include "ui_desktopnotification.h"

DesktopNotification::DesktopNotification(bool settingPosition)
   : QWidget(0)
   , ui(new Ui::DesktopNotification)
   , m_settingPosition(settingPosition)
   , m_timeout(6000)
   , m_timer(new QTimer(this))
{
    ui->setupUi(this);
    setStyleSheet("background:transparent;");
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    Qt::WindowFlags flags = Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
                            Qt::X11BypassWindowManagerHint;
  #ifdef Q_WS_WIN
    flags |= Qt::ToolTip;
  #endif
    setWindowFlags(flags);
    setWindowOpacity(0.9);

    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(close()));
}

void DesktopNotification::show()
{
    ui->icon->setPixmap(m_icon);
    ui->heading->setText(m_heading);
    ui->text->setText(m_text);

    if (!m_settingPosition) {
        m_timer->stop();
        m_timer->setInterval(m_timeout);
        m_timer->start();
    }

    QWidget::show();
}

void DesktopNotification::enterEvent(QEvent *e)
{
    Q_UNUSED(e)
    setWindowOpacity(0.5);
}

void DesktopNotification::leaveEvent(QEvent *e)
{
    Q_UNUSED(e)
    setWindowOpacity(0.9);
}

void DesktopNotification::mousePressEvent(QMouseEvent *e)
{
    if (!m_settingPosition) {
        close();
        return;
    }

    if (e->button() == Qt::LeftButton) {
        m_dragPosition = e->globalPos() - frameGeometry().topLeft();
        e->accept();
    }
}

void DesktopNotification::mouseMoveEvent(QMouseEvent *e)
{
    if (e->buttons() & Qt::LeftButton) {
        move(e->globalPos() - m_dragPosition);
        e->accept();
    }
}

DesktopNotification::~DesktopNotification()
{
    delete ui;
}
