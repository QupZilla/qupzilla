#ifndef DESKTOPNOTIFICATION_H
#define DESKTOPNOTIFICATION_H

#include <QWidget>
#include <QTimer>
#include <QMouseEvent>

namespace Ui {
    class DesktopNotification;
}

class DesktopNotification : public QWidget
{
    Q_OBJECT

public:
    explicit DesktopNotification(bool settingPosition = false);
    void setPixmap(const QPixmap &icon) { m_icon = icon; }
    void setHeading(const QString &heading) { m_heading = heading; }
    void setText(const QString &text) { m_text = text; }
    void setTimeout(int timeout) { m_timeout = timeout; }
    void show();
    ~DesktopNotification();

private:
    void enterEvent(QEvent* e);
    void leaveEvent(QEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);

    Ui::DesktopNotification* ui;
    bool m_settingPosition;
    QPoint m_dragPosition;

    QPixmap m_icon;
    QString m_heading;
    QString m_text;
    int m_timeout;
    QTimer* m_timer;
};

#endif // DESKTOPNOTIFICATION_H
