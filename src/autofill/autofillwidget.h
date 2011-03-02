#ifndef AUTOFILLWIDGET_H
#define AUTOFILLWIDGET_H

#include <QWidget>
#include <QUrl>
#include <QTimeLine>

namespace Ui {
    class AutoFillWidget;
}

class AutoFillWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AutoFillWidget(QUrl url, QByteArray data, QString pass, QWidget *parent = 0);
    ~AutoFillWidget();

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
