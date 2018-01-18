/* ============================================================
* QupZilla - WebKit based browser
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
#ifndef ACCEPTLANGUAGE_H
#define ACCEPTLANGUAGE_H

#include <QDialog>
#include <QLocale>

#include "qzcommon.h"

namespace Ui
{
class AcceptLanguage;
}

class QUPZILLA_EXPORT AcceptLanguage : public QDialog
{
    Q_OBJECT

public:
    explicit AcceptLanguage(QWidget* parent = 0);
    ~AcceptLanguage();

    static QStringList defaultLanguage();
    static QByteArray generateHeader(const QStringList &langs);

public slots:
    void accept();

private slots:
    void addLanguage();
    void removeLanguage();
    void upLanguage();
    void downLanguage();

private:
    QStringList expand(const QLocale::Language &language);

    Ui::AcceptLanguage* ui;
};

#endif // ACCEPTLANGUAGE_H
