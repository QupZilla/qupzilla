#ifndef RSSWIDGET_H
#define RSSWIDGET_H

#include <QWidget>
#include <QMenu>
#include <QPushButton>
#include <QDebug>

namespace Ui {
    class RSSWidget;
}

class WebView;
class RSSWidget : public QMenu
{
    Q_OBJECT

public:
    explicit RSSWidget(WebView* view, QList<QPair<QString,QString> > availableRss, QWidget *parent = 0);
    ~RSSWidget();

    void showAt(QWidget* _parent);

private slots:
    void addRss();

private:
    Ui::RSSWidget *ui;
    QList<QPair<QString,QString> > m_avRss;
    WebView* m_view;
};

#endif // RSSWIDGET_H
