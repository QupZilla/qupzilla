#include "rsswidget.h"
#include "ui_rsswidget.h"

RSSWidget::RSSWidget(QList<QPair<QString, QString> > availableRss, QWidget *parent)
    :QMenu(parent)
    ,ui(new Ui::RSSWidget)
    ,m_avRss(availableRss)
{
    ui->setupUi(this);
}

void RSSWidget::showAt(QWidget* _parent)
{
    QPoint p = _parent->mapToGlobal(QPoint(0, 0));
    move( (p.x()+_parent->width() ) - width(), p.y() + _parent->height());
    show();
}

RSSWidget::~RSSWidget()
{
    delete ui;
}
