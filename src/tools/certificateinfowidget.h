#ifndef CERTIFICATEINFOWIDGET_H
#define CERTIFICATEINFOWIDGET_H

#include <QWidget>
#include <QSslCertificate>
#include <QDateTime>

namespace Ui {
    class CertificateInfoWidget;
}

class CertificateInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CertificateInfoWidget(const QSslCertificate &cert, QWidget *parent = 0);
    ~CertificateInfoWidget();

    static QString showCertInfo(const QString &string);

private:
    Ui::CertificateInfoWidget *ui;
};

#endif // CERTIFICATEINFOWIDGET_H
