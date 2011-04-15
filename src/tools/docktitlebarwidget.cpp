#include "docktitlebarwidget.h"

DockTitleBarWidget::DockTitleBarWidget(const QString &title, QWidget* parent) :
    QWidget(parent)
{
    setupUi(this);
#ifdef Q_WS_X11
    closeButton->setIcon(QIcon(style()->standardIcon(QStyle::SP_DialogCloseButton).pixmap(16,16)));
#else
    closeButton->setIcon(QIcon(QIcon(":/icons/faenza/close.png").pixmap(16,16)));
#endif
    label->setText(title);
    connect(closeButton, SIGNAL(clicked()), parent, SLOT(close()));
}

void DockTitleBarWidget::setTitle(const QString &title)
{
    label->setText(title);
}

DockTitleBarWidget::~DockTitleBarWidget()
{
}
