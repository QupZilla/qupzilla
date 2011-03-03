#include "autofillnotification.h"
#include "ui_autofillnotification.h"
#include "autofillmodel.h"
#include "mainapplication.h"

AutoFillNotification::AutoFillNotification(QUrl url, QByteArray data, QString pass, QWidget *parent)
   :QWidget(parent)
   ,ui(new Ui::AutoFillWidget)
   ,m_url(url)
   ,m_data(data)
   ,m_pass(pass)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);
    ui->label->setText(tr("Do you want QupZilla to remember password on %1?").arg(url.host()));
    ui->closeButton->setIcon(
#ifdef Q_WS_X11
    style()->standardIcon(QStyle::SP_DialogCloseButton)
#else
    QIcon(":/icons/faenza/close.png")
#endif
    );

    connect(ui->remember, SIGNAL(clicked()), this, SLOT(remember()));
    connect(ui->never, SIGNAL(clicked()), this, SLOT(never()));
    connect(ui->notnow, SIGNAL(clicked()), this, SLOT(hide()));
    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(hide()));

    setMinimumHeight(1);
    setMaximumHeight(1);

    m_animation = new QTimeLine(300, this);
    m_animation->setFrameRange(0, 35);
    connect(m_animation, SIGNAL(frameChanged(int)),this, SLOT(frameChanged(int)));
    QTimer::singleShot(300, m_animation, SLOT(start()));
}

void AutoFillNotification::hide()
{
    m_animation->setDirection(QTimeLine::Backward);

    m_animation->stop();
    m_animation->start();
    connect(m_animation, SIGNAL(finished()), this, SLOT(close()));
}

void AutoFillNotification::frameChanged(int frame)
{
    setMinimumHeight(frame);
    setMaximumHeight(frame);
}

void AutoFillNotification::never()
{
    MainApplication::getInstance()->autoFill()->blockStoringfor(m_url);
    hide();
}

void AutoFillNotification::remember()
{
    MainApplication::getInstance()->autoFill()->addEntry(m_url, m_data, m_pass);
    hide();
}

AutoFillNotification::~AutoFillNotification()
{
    delete ui;
}
