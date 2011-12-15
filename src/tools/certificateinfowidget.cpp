/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca <nowrep@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
* ============================================================ */
#include "certificateinfowidget.h"
#include "ui_certificateinfowidget.h"

QString CertificateInfoWidget::certificateItemText(const QSslCertificate &cert)
{
    QString commonName = cert.subjectInfo(QSslCertificate::CommonName);
    QString organization = cert.subjectInfo(QSslCertificate::Organization);

    if (commonName.isEmpty()) {
        return clearCertSpecialSymbols(organization);
    }

    return clearCertSpecialSymbols(commonName);
}

QString CertificateInfoWidget::clearCertSpecialSymbols(const QString &string)
{
    QString n = Qt::escape(string);

    if (!n.contains("\\")) {
        return n;
    }

    //Credits to http://blade.nagaokaut.ac.jp/cgi-bin/scat.rb/ruby/ruby-talk/176679?help-en
    n.replace("\\xC3\\x80", "A");
    n.replace("\\xC3\\x81", "A");
    n.replace("\\xC3\\x82", "A");
    n.replace("\\xC3\\x83", "A");
    n.replace("\\xC3\\x84", "A");
    n.replace("\\xC3\\x85", "A");
    n.replace("\\xC3\\x86", "AE");
    n.replace("\\xC3\\x87", "C");
    n.replace("\\xC3\\x88", "E");
    n.replace("\\xC3\\x89", "E");
    n.replace("\\xC3\\x8A", "E");
    n.replace("\\xC3\\x8B", "E");
    n.replace("\\xC3\\x8C", "I");
    n.replace("\\xC3\\x8D", "I");
    n.replace("\\xC3\\x8E", "I");
    n.replace("\\xC3\\x8F", "I");
    n.replace("\\xC3\\x90", "D");
    n.replace("\\xC3\\x91", "N");
    n.replace("\\xC3\\x92", "O");
    n.replace("\\xC3\\x93", "O");
    n.replace("\\xC3\\x94", "O");
    n.replace("\\xC3\\x95", "O");
    n.replace("\\xC3\\x96", "O");
    n.replace("\\xC3\\x98", "O");
    n.replace("\\xC3\\x99", "U");
    n.replace("\\xC3\\x9A", "U");
    n.replace("\\xC3\\x9B", "U");
    n.replace("\\xC3\\x9C", "U");
    n.replace("\\xC3\\x9D", "Y");
    n.replace("\\xC3\\x9E", "P");
    n.replace("\\xC3\\x9F", "ss");
    n.replace("\\xC9\\x99", "e");
    n.replace("\\xC3\\xA0", "a");
    n.replace("\\xC3\\xA1", "a");
    n.replace("\\xC3\\xA2", "a");
    n.replace("\\xC3\\xA3", "a");
    n.replace("\\xC3\\xA4", "a");
    n.replace("\\xC3\\xA5", "a");
    n.replace("\\xC3\\xA6", "ae");
    n.replace("\\xC3\\xA7", "c");
    n.replace("\\xC3\\xA8", "e");
    n.replace("\\xC3\\xA9", "e");
    n.replace("\\xC3\\xAA", "e");
    n.replace("\\xC3\\xAB", "e");
    n.replace("\\xC3\\xAC", "i");
    n.replace("\\xC3\\xAD", "i");
    n.replace("\\xC3\\xAE", "i");
    n.replace("\\xC3\\xAF", "i");
    n.replace("\\xC3\\xB0", "o");
    n.replace("\\xC3\\xB1", "n");
    n.replace("\\xC3\\xB2", "o");
    n.replace("\\xC3\\xB3", "o");
    n.replace("\\xC3\\xB4", "o");
    n.replace("\\xC3\\xB5", "o");
    n.replace("\\xC3\\xB6", "o");
    n.replace("\\xC3\\xB8", "o");
    n.replace("\\xC3\\xB9", "u");
    n.replace("\\xC3\\xBA", "u");
    n.replace("\\xC3\\xBB", "u");
    n.replace("\\xC3\\xBC", "u");
    n.replace("\\xC3\\xBD", "y");
    n.replace("\\xC3\\xBE", "p");
    n.replace("\\xC3\\xBF", "y");
    n.replace("\\xC7\\xBF", "o");
    n.replace("\\xC4\\x80", "A");
    n.replace("\\xC4\\x81", "a");
    n.replace("\\xC4\\x82", "A");
    n.replace("\\xC4\\x83", "a");
    n.replace("\\xC4\\x84", "A");
    n.replace("\\xC4\\x85", "a");
    n.replace("\\xC4\\x86", "C");
    n.replace("\\xC4\\x87", "c");
    n.replace("\\xC4\\x88", "C");
    n.replace("\\xC4\\x89", "c");
    n.replace("\\xC4\\x8A", "C");
    n.replace("\\xC4\\x8B", "c");
    n.replace("\\xC4\\x8C", "C");
    n.replace("\\xC4\\x8D", "c");
    n.replace("\\xC4\\x8E", "D");
    n.replace("\\xC4\\x8F", "d");
    n.replace("\\xC4\\x90", "D");
    n.replace("\\xC4\\x91", "d");
    n.replace("\\xC4\\x92", "E");
    n.replace("\\xC4\\x93", "e");
    n.replace("\\xC4\\x94", "E");
    n.replace("\\xC4\\x95", "e");
    n.replace("\\xC4\\x96", "E");
    n.replace("\\xC4\\x97", "e");
    n.replace("\\xC4\\x98", "E");
    n.replace("\\xC4\\x99", "e");
    n.replace("\\xC4\\x9A", "E");
    n.replace("\\xC4\\x9B", "e");
    n.replace("\\xC4\\x9C", "G");
    n.replace("\\xC4\\x9D", "g");
    n.replace("\\xC4\\x9E", "G");
    n.replace("\\xC4\\x9F", "g");
    n.replace("\\xC4\\xA0", "G");
    n.replace("\\xC4\\xA1", "g");
    n.replace("\\xC4\\xA2", "G");
    n.replace("\\xC4\\xA3", "g");
    n.replace("\\xC4\\xA4", "H");
    n.replace("\\xC4\\xA5", "h");
    n.replace("\\xC4\\xA6", "H");
    n.replace("\\xC4\\xA7", "h");
    n.replace("\\xC4\\xA8", "I");
    n.replace("\\xC4\\xA9", "i");
    n.replace("\\xC4\\xAA", "I");
    n.replace("\\xC4\\xAB", "i");
    n.replace("\\xC4\\xAC", "I");
    n.replace("\\xC4\\xAD", "i");
    n.replace("\\xC4\\xAE", "I");
    n.replace("\\xC4\\xAF", "i");
    n.replace("\\xC4\\xB0", "I");
    n.replace("\\xC4\\xB1", "i");
    n.replace("\\xC4\\xB2", "IJ");
    n.replace("\\xC4\\xB3", "ij");
    n.replace("\\xC4\\xB4", "J");
    n.replace("\\xC4\\xB5", "j");
    n.replace("\\xC4\\xB6", "K");
    n.replace("\\xC4\\xB7", "k");
    n.replace("\\xC4\\xB8", "k");
    n.replace("\\xC4\\xB9", "L");
    n.replace("\\xC4\\xBA", "l");
    n.replace("\\xC4\\xBB", "L");
    n.replace("\\xC4\\xBC", "l");
    n.replace("\\xC4\\xBD", "L");
    n.replace("\\xC4\\xBE", "l");
    n.replace("\\xC4\\xBF", "L");
    n.replace("\\xC5\\x80", "l");
    n.replace("\\xC5\\x81", "L");
    n.replace("\\xC5\\x82", "l");
    n.replace("\\xC5\\x83", "N");
    n.replace("\\xC5\\x84", "n");
    n.replace("\\xC5\\x85", "N");
    n.replace("\\xC5\\x86", "n");
    n.replace("\\xC5\\x87", "N");
    n.replace("\\xC5\\x88", "n");
    n.replace("\\xC5\\x89", "n");
    n.replace("\\xC5\\x8A", "N");
    n.replace("\\xC5\\x8B", "n");
    n.replace("\\xC5\\x8C", "O");
    n.replace("\\xC5\\x8D", "o");
    n.replace("\\xC5\\x8E", "O");
    n.replace("\\xC5\\x8F", "o");
    n.replace("\\xC5\\x90", "O");
    n.replace("\\xC5\\x91", "o");
    n.replace("\\xC5\\x92", "CE");
    n.replace("\\xC5\\x93", "ce");
    n.replace("\\xC5\\x94", "R");
    n.replace("\\xC5\\x95", "r");
    n.replace("\\xC5\\x96", "R");
    n.replace("\\xC5\\x97", "r");
    n.replace("\\xC5\\x98", "R");
    n.replace("\\xC5\\x99", "r");
    n.replace("\\xC5\\x9A", "S");
    n.replace("\\xC5\\x9B", "s");
    n.replace("\\xC5\\x9C", "S");
    n.replace("\\xC5\\x9D", "s");
    n.replace("\\xC5\\x9E", "S");
    n.replace("\\xC5\\x9F", "s");
    n.replace("\\xC5\\xA0", "S");
    n.replace("\\xC5\\xA1", "s");
    n.replace("\\xC5\\xA2", "T");
    n.replace("\\xC5\\xA3", "t");
    n.replace("\\xC5\\xA4", "T");
    n.replace("\\xC5\\xA5", "t");
    n.replace("\\xC5\\xA6", "T");
    n.replace("\\xC5\\xA7", "t");
    n.replace("\\xC5\\xA8", "U");
    n.replace("\\xC5\\xA9", "u");
    n.replace("\\xC5\\xAA", "U");
    n.replace("\\xC5\\xAB", "u");
    n.replace("\\xC5\\xAC", "U");
    n.replace("\\xC5\\xAD", "u");
    n.replace("\\xC5\\xAE", "U");
    n.replace("\\xC5\\xAF", "u");
    n.replace("\\xC5\\xB0", "U");
    n.replace("\\xC5\\xB1", "u");
    n.replace("\\xC5\\xB2", "U");
    n.replace("\\xC5\\xB3", "u");
    n.replace("\\xC5\\xB4", "W");
    n.replace("\\xC5\\xB5", "w");
    n.replace("\\xC5\\xB6", "Y");
    n.replace("\\xC5\\xB7", "y");
    n.replace("\\xC5\\xB8", "Y");
    n.replace("\\xC5\\xB9", "Z");
    n.replace("\\xC5\\xBA", "z");
    n.replace("\\xC5\\xBB", "Z");
    n.replace("\\xC5\\xBC", "z");
    n.replace("\\xC5\\xBD", "Z");
    n.replace("\\xC5\\xBE", "z");
    n.replace("\\xC6\\x8F", "E");
    n.replace("\\xC6\\xA0", "O");
    n.replace("\\xC6\\xA1", "o");
    n.replace("\\xC6\\xAF", "U");
    n.replace("\\xC6\\xB0", "u");
    n.replace("\\xC7\\x8D", "A");
    n.replace("\\xC7\\x8E", "a");
    n.replace("\\xC7\\x8F", "I");
    n.replace("\\xC7\\x93", "U");
    n.replace("\\xC7\\x90", "i");
    n.replace("\\xC7\\x91", "O");
    n.replace("\\xC7\\x92", "o");
    n.replace("\\xC7\\x97", "U");
    n.replace("\\xC7\\x94", "u");
    n.replace("\\xC7\\x95", "U");
    n.replace("\\xC7\\x96", "u");
    n.replace("\\xC7\\x9B", "U");
    n.replace("\\xC7\\x98", "u");
    n.replace("\\xC7\\x99", "U");
    n.replace("\\xC7\\x9A", "u");
    n.replace("\\xC7\\xBD", "ae");
    n.replace("\\xC7\\x9C", "u");
    n.replace("\\xC7\\xBB", "a");
    n.replace("\\xC7\\xBC", "AE");
    n.replace("\\xC7\\xBE", "O");
    n.replace("\\xC7\\xBA", "A");

    n.replace("\\xC2\\x82", ",");        // High code comma
    n.replace("\\xC2\\x84", ",,");       // High code double comma
    n.replace("\\xC2\\x85", "...");      // Tripple dot
    n.replace("\\xC2\\x88", "^");        // High carat
    n.replace("\\xC2\\x91", "\\x27");    // Forward single quote
    n.replace("\\xC2\\x92", "\\x27");    // Reverse single quote
    n.replace("\\xC2\\x93", "\\x22");    // Forward double quote
    n.replace("\\xC2\\x94", "\\x22");    // Reverse double quote
    n.replace("\\xC2\\x96", "-");        // High hyphen
    n.replace("\\xC2\\x97", "--");       // Double hyphen
    n.replace("\\xC2\\xA6", "|");        // Split vertical bar
    n.replace("\\xC2\\xAB", "<<");       // Double less than
    n.replace("\\xC2\\xBB", ">>");       // Double greater than
    n.replace("\\xC2\\xBC", "1/4");      // one quarter
    n.replace("\\xC2\\xBD", "1/2");      // one half
    n.replace("\\xC2\\xBE", "3/4");      // three quarters
    n.replace("\\xCA\\xBF", "\\x27");    // c-single quote
    n.replace("\\xCC\\xA8", "");         // modifier - under curve
    n.replace("\\xCC\\xB1", "");         // modifier - under line

    return n;
}

QString CertificateInfoWidget::showCertInfo(const QString &string)
{
    if (string.isEmpty()) {
        return tr("<not set in certificate>");
    }
    else {
        return clearCertSpecialSymbols(string);
    }
}

CertificateInfoWidget::CertificateInfoWidget(const QSslCertificate &cert, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::CertificateInfoWidget)
{
    ui->setupUi(this);

    //Issued to
    ui->issuedToCN->setText(showCertInfo(cert.subjectInfo(QSslCertificate::CommonName)));
    ui->issuedToO->setText(showCertInfo(cert.subjectInfo(QSslCertificate::Organization)));
    ui->issuedToOU->setText(showCertInfo(cert.subjectInfo(QSslCertificate::OrganizationalUnitName)));
    ui->issuedToSN->setText(showCertInfo(cert.serialNumber()));
    //Issued By
    ui->issuedByCN->setText(showCertInfo(cert.issuerInfo(QSslCertificate::CommonName)));
    ui->issuedByO->setText(showCertInfo(cert.issuerInfo(QSslCertificate::Organization)));
    ui->issuedByOU->setText(showCertInfo(cert.issuerInfo(QSslCertificate::OrganizationalUnitName)));
    //Validity
    ui->validityIssuedOn->setText(cert.effectiveDate().toString("dddd d. MMMM yyyy"));
    ui->validityExpiresOn->setText(cert.expiryDate().toString("dddd d. MMMM yyyy"));
}

CertificateInfoWidget::~CertificateInfoWidget()
{
    delete ui;
}
