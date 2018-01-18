/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018  David Rosca <nowrep@gmail.com>
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

#include "qzcommon.h"

namespace Ui
{
class CertificateInfoWidget;
}

class QSslCertificate;

class QUPZILLA_EXPORT CertificateInfoWidget : public QWidget
{
public:
    explicit CertificateInfoWidget(const QSslCertificate &cert, QWidget* parent = 0);
    ~CertificateInfoWidget();

    // Qt5 compatibility
    // QSslCertificate::subjectInfo returns:
    //      QString     in Qt 4
    //      QStringList in Qt 5
    //
    static QString showCertInfo(const QString &string);
    static QString showCertInfo(const QStringList &stringList);
    static QString clearCertSpecialSymbols(const QString &string);
    static QString clearCertSpecialSymbols(const QStringList &stringList);

    static QString certificateItemText(const QSslCertificate &cert);

private:
    Ui::CertificateInfoWidget* ui;
};

#endif // CERTIFICATEINFOWIDGET_H
