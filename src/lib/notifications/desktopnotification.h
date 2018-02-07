/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018 David Rosca <nowrep@gmail.com>
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
#ifndef DESKTOPNOTIFICATION_H
#define DESKTOPNOTIFICATION_H

#include "qzcommon.h"

#include <QWidget>
#include <QPixmap>

namespace Ui
{
class DesktopNotification;
}

class QUPZILLA_EXPORT DesktopNotification : public QWidget
{
    Q_OBJECT

public:
    explicit DesktopNotification(bool setPosition = false);
    void setPixmap(const QPixmap &icon) { m_icon = icon; }
    void setHeading(const QString &heading) { m_heading = heading; }
    void setText(const QString &text) { m_text = text; }
    void setTimeout(int timeout) { m_timeout = timeout; }
    void show();
    ~DesktopNotification();

private:
    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);

    Ui::DesktopNotification* ui;
    bool m_settingPosition;
    QPoint m_dragPosition;

    QPixmap m_icon;
    QString m_heading;
    QString m_text;
    int m_timeout;
    QTimer* m_timer;
};

#endif // DESKTOPNOTIFICATION_H
