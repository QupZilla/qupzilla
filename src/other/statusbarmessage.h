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
#ifndef STATUSBARMESSAGE_H
#define STATUSBARMESSAGE_H

#include <QObject>
#include <QToolTip>
#include "squeezelabelv1.h"
#include "animatedwidget.h"

class QupZilla;
class TipLabel;

class TipLabel : public SqueezeLabelV1 {
    Q_OBJECT

public:
    TipLabel(QupZilla* parent);

    void show();

private:
    void paintEvent(QPaintEvent* ev);

    QupZilla* p_QupZilla;
    bool m_connected;
};

class StatusBarMessage : public QObject
{
    Q_OBJECT
public:
    explicit StatusBarMessage(QupZilla* mainClass);

    void showMessage(const QString &message);
    void clearMessage();

signals:

public slots:

private:
    QupZilla* p_QupZilla;
    TipLabel* m_statusBarText;
};

#endif // STATUSBARMESSAGE_H
