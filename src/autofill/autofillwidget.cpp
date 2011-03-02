#include "autofillwidget.h"
#include "ui_autofillwidget.h"
#include "autofillmodel.h"
#include "mainapplication.h"

AutoFillWidget::AutoFillWidget(QUrl url, QByteArray data, QString pass, QWidget *parent)
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
    connect(ui->notnow, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(close()));
}
//    addEntry(request.url(), outgoingData, pass);

void AutoFillWidget::never()
{
    MainApplication::getInstance()->autoFill()->blockStoringfor(m_url);
    close();
}

void AutoFillWidget::remember()
{
    MainApplication::getInstance()->autoFill()->addEntry(m_url, m_data, m_pass);
    close();
}

AutoFillWidget::~AutoFillWidget()
{
    delete ui;
}
