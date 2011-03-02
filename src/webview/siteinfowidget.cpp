#include "siteinfowidget.h"
#include "ui_siteinfowidget.h"
#include "qupzilla.h"

SiteInfoWidget::SiteInfoWidget(QupZilla* mainClass, QWidget *parent) :
    QMenu(parent)
    ,ui(new Ui::SiteInfoWidget)
    ,p_QupZilla(mainClass)
{
    QUrl url = p_QupZilla->weView()->url();
    if (url.isEmpty())
        return;

    this->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    QString scheme = url.scheme();
    if (scheme == "https") {
        ui->secureLabel->setText(tr("Your connection to this site is <b>secured</b>."));
        ui->secureIcon->setPixmap(QPixmap(":/icons/locationbar/accept.png"));
    }
    else {
        ui->secureLabel->setText(tr("Your connection to this site is <b>unsecured</b>."));
        ui->secureIcon->setPixmap(QPixmap(":/icons/locationbar/warning.png"));
    }

    QSqlQuery query;
    QString host = url.host();
    QString host2 = host;
    if (host.startsWith("www."))
        host2 = url.host().remove("www.");

    query.exec("SELECT sum(count) FROM history WHERE url LIKE '"+scheme+"://"+host+"%' ");
    if (query.next()) {
        int count = query.value(0).toInt();
        if (count > 3) {
            ui->historyLabel->setText(tr("This is Your <b>%1.</b> visit of this site.").arg(count));
            ui->historyIcon->setPixmap(QPixmap(":/icons/locationbar/accept.png"));
        } else if (count == 0) {
                ui->historyLabel->setText(tr("You have <b>never</b> visited this site before."));
                ui->historyIcon->setPixmap(QPixmap(":/icons/locationbar/warning.png"));
        } else {
            ui->historyIcon->setPixmap(QPixmap(":/icons/locationbar/warning.png"));
            QString text;
            if (count == 1)
                text = tr("first");
            else if (count == 2)
                text = tr("second");
            else if (count == 3)
                text = tr("third");
            ui->historyLabel->setText(tr("This is Your <b>%1</b> visit of this site.").arg(text));
        }
    }
    connect(ui->pushButton, SIGNAL(clicked()), p_QupZilla, SLOT(showPageInfo()));
}

void SiteInfoWidget::showAt(QWidget* _parent)
{
    QPoint p = _parent->mapToGlobal(QPoint(0, 0));
    move(p.x(), p.y() + _parent->height());
    show();
}

SiteInfoWidget::~SiteInfoWidget()
{
    delete ui;
}
