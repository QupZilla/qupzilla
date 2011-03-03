#include "rssnotification.h"
#include "ui_rssnotification.h"
#include "mainapplication.h"
#include "qupzilla.h"

RSSNotification::RSSNotification(QString host, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RSSNotification)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);
    setMinimumHeight(1);
    setMaximumHeight(1);
    ui->closeButton->setIcon(
#ifdef Q_WS_X11
    style()->standardIcon(QStyle::SP_DialogCloseButton)
#else
    QIcon(":/icons/faenza/close.png")
#endif
    );
    ui->label->setText(tr("You have successfuly added RSS feed \"%1\".").arg(host));

    connect(ui->pushButton, SIGNAL(clicked()), MainApplication::getInstance()->getWindow(), SLOT(showRSSManager()));
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(hide()));
    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(hide()));

    m_animation = new QTimeLine(300, this);
    m_animation->setFrameRange(0, 35);
    connect(m_animation, SIGNAL(frameChanged(int)),this, SLOT(frameChanged(int)));
    QTimer::singleShot(1, m_animation, SLOT(start()));
}

void RSSNotification::hide()
{
    m_animation->setDirection(QTimeLine::Backward);

    m_animation->stop();
    m_animation->start();
    connect(m_animation, SIGNAL(finished()), this, SLOT(close()));
}

void RSSNotification::frameChanged(int frame)
{
    setMinimumHeight(frame);
    setMaximumHeight(frame);
}

RSSNotification::~RSSNotification()
{
    delete ui;
}
