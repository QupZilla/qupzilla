#ifndef AUTOFILLWIDGET_H
#define AUTOFILLWIDGET_H

#include <QWidget>
#include <QUrl>
#include <QTimeLine>
#include <QTimer>

namespace Ui {
    class AutoFillWidget;
}

class AutoFillNotification : public QWidget
{
    Q_OBJECT

public:
    explicit AutoFillNotification(QUrl url, QByteArray data, QString pass, QWidget *parent = 0);
    ~AutoFillNotification();

private slots:
    void frameChanged(int frame);
    void remember();
    void never();

    void hide();

private:
    Ui::AutoFillWidget *ui;
    QUrl m_url;
    QByteArray m_data;
    QString m_pass;
    QTimeLine* m_animation;
};

#endif // AUTOFILLWIDGET_H
