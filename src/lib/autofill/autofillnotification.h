/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
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
#ifndef AUTOFILLWIDGET_H
#define AUTOFILLWIDGET_H

#include <QUrl>

#include "qz_namespace.h"
#include "animatedwidget.h"

namespace Ui
{
class AutoFillWidget;
}

class AnimatedWidget;
class QT_QUPZILLA_EXPORT AutoFillNotification : public AnimatedWidget
{
    Q_OBJECT

public:
    explicit AutoFillNotification(const QUrl &url, const QByteArray &data, const QString &user, const QString &pass, QWidget* parent = 0);
    ~AutoFillNotification();

private slots:
    void remember();
    void never();

private:
    Ui::AutoFillWidget* ui;
    QUrl m_url;
    QByteArray m_data;
    QString m_user;
    QString m_pass;
};

#endif // AUTOFILLWIDGET_H
