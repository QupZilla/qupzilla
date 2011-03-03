#ifndef RSSNOTIFICATION_H
#define RSSNOTIFICATION_H

#include <QWidget>
#include <QTimeLine>

namespace Ui {
    class RSSNotification;
}

class RSSNotification : public QWidget
{
    Q_OBJECT

public:
    explicit RSSNotification(QString host, QWidget *parent = 0);
    ~RSSNotification();

private slots:
    void hide();
    void frameChanged(int frame);

private:
    Ui::RSSNotification *ui;
    QTimeLine* m_animation;
};

#endif // RSSNOTIFICATION_H
