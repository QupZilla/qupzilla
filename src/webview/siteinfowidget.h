#ifndef SITEINFOWIDGET_H
#define SITEINFOWIDGET_H

#include <QWidget>
#include <QMenu>

namespace Ui {
    class SiteInfoWidget;
}

class QupZilla;
class SiteInfoWidget : public QMenu
{
    Q_OBJECT

public:
    explicit SiteInfoWidget(QupZilla* mainClass, QWidget *parent = 0);
    ~SiteInfoWidget();

    void showAt(QWidget* _parent);

private:
    Ui::SiteInfoWidget *ui;
    QupZilla* p_QupZilla;
};

#endif // SITEINFOWIDGET_H
