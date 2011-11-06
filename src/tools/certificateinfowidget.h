/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca
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
#ifndef CERTIFICATEINFOWIDGET_H
#define CERTIFICATEINFOWIDGET_H

#include <QWidget>
#include <QSslCertificate>
#include <QDateTime>
#include <QTextDocument>

namespace Ui
{
class CertificateInfoWidget;
}

class CertificateInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CertificateInfoWidget(const QSslCertificate &cert, QWidget* parent = 0);
    ~CertificateInfoWidget();

    static QString showCertInfo(const QString &string);
    static QString clearCertSpecialSymbols(const QString &string);
    static QString certificateItemText(const QSslCertificate &cert);

private:
    Ui::CertificateInfoWidget* ui;
};

#endif // CERTIFICATEINFOWIDGET_H
