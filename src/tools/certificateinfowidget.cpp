#include "certificateinfowidget.h"
#include "ui_certificateinfowidget.h"

QString CertificateInfoWidget::showCertInfo(const QString &string)
{
    if (string.isEmpty())
        return tr("<not set in certificate>");
    else return string;
}

CertificateInfoWidget::CertificateInfoWidget(const QSslCertificate &cert, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CertificateInfoWidget)
{
    ui->setupUi(this);

    //Issued to
    ui->issuedToCN->setText( showCertInfo(cert.subjectInfo(QSslCertificate::CommonName)) );
    ui->issuedToO->setText( showCertInfo(cert.subjectInfo(QSslCertificate::Organization)) );
    ui->issuedToOU->setText( showCertInfo(cert.subjectInfo(QSslCertificate::OrganizationalUnitName)) );
    ui->issuedToSN->setText( showCertInfo(cert.serialNumber()) );
    //Issued By
    ui->issuedByCN->setText( showCertInfo(cert.issuerInfo(QSslCertificate::CommonName)) );
    ui->issuedByO->setText( showCertInfo(cert.issuerInfo(QSslCertificate::Organization)) );
    ui->issuedByOU->setText( showCertInfo(cert.issuerInfo(QSslCertificate::OrganizationalUnitName)) );
    //Validity
    ui->validityIssuedOn->setText( cert.effectiveDate().toString("dddd d. MMMM yyyy") );
    ui->validityExpiresOn->setText( cert.expiryDate().toString("dddd d. MMMM yyyy") );
}

CertificateInfoWidget::~CertificateInfoWidget()
{
    delete ui;
}
