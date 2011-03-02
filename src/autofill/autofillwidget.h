#ifndef AUTOFILLWIDGET_H
#define AUTOFILLWIDGET_H

#include <QWidget>
#include <QUrl>

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
    void remember();
    void never();

private:
    Ui::AutoFillWidget *ui;
    QUrl m_url;
    QByteArray m_data;
    QString m_pass;
};

#endif // AUTOFILLWIDGET_H
