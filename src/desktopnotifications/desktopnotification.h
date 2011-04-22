#ifndef DESKTOPNOTIFICATION_H
#define DESKTOPNOTIFICATION_H

#include <QWidget>
#include <QTimer>

namespace Ui {
    class DesktopNotification;
}

class DesktopNotification : public QWidget
{
    Q_OBJECT

public:
    explicit DesktopNotification(const QPixmap &icon, const QString &heading, const QString &text, int timeout);
    ~DesktopNotification();

private:
    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);
    void mousePressEvent(QMouseEvent *e);

    Ui::DesktopNotification *ui;
};

#endif // DESKTOPNOTIFICATION_H
