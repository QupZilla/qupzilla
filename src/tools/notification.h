#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <QWidget>
#include <QTimeLine>
#include <QTimer>

class Notification : public QWidget
{
    Q_OBJECT
public:
    explicit Notification(QWidget* parent = 0);

public slots:
    void hide();
    void startAnimation();

private slots:
    void frameChanged(int frame);

private:
    QTimeLine* m_animation;
};

#endif // NOTIFICATION_H
