#ifndef RSSWIDGET_H
#define RSSWIDGET_H

#include <QWidget>
#include <QMenu>

namespace Ui {
    class RSSWidget;
}

class RSSWidget : public QMenu
{
    Q_OBJECT

public:
    explicit RSSWidget(QList<QPair<QString,QString> > availableRss, QWidget *parent = 0);
    ~RSSWidget();

    void showAt(QWidget* _parent);

private:
    Ui::RSSWidget *ui;
    QList<QPair<QString,QString> > m_avRss;
};

#endif // RSSWIDGET_H
