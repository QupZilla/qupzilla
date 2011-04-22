#include "desktopnotification.h"
#include "ui_desktopnotification.h"

DesktopNotification::DesktopNotification(const QPixmap &icon, const QString &heading, const QString &text, int timeout)
   : QWidget(0)
   , ui(new Ui::DesktopNotification)
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

    ui->icon->setPixmap(icon);
    ui->heading->setText(QString("<b>%1</b>").arg(heading));
    ui->text->setText(text);

    QTimer::singleShot(timeout, this, SLOT(close()));
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
    Q_UNUSED(e)
    close();
}

DesktopNotification::~DesktopNotification()
{
    delete ui;
}
