/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  nowrep
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

#include <QWidget>
#include <QUrl>
#include <QTimer>
#include <QDebug>

#include "notification.h"

namespace Ui {
    class AutoFillWidget;
}
class Notification;
class AutoFillNotification : public Notification
{
    Q_OBJECT

public:
    explicit AutoFillNotification(QUrl url, QByteArray data, QString pass, QWidget* parent = 0);
    ~AutoFillNotification();

private slots:
    void remember();
    void never();

private:
    Ui::AutoFillWidget* ui;
    QUrl m_url;
    QByteArray m_data;
    QString m_pass;
    QTimeLine* m_animation;
};

#endif // AUTOFILLWIDGET_H
