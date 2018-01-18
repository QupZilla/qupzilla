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
#ifndef STATUSBARMESSAGE_H
#define STATUSBARMESSAGE_H

#include <QObject>

#include "qzcommon.h"
#include "squeezelabelv1.h"
#include "animatedwidget.h"

class QTimer;

class BrowserWindow;

class QUPZILLA_EXPORT TipLabel : public SqueezeLabelV1
{
public:
    explicit TipLabel(QWidget* parent);

    void show(QWidget* widget);
    void hideDelayed();

    bool eventFilter(QObject* o, QEvent* e);

private:
    void resizeEvent(QResizeEvent* ev);
    void paintEvent(QPaintEvent* ev);

    QTimer* m_timer;
};

class QUPZILLA_EXPORT StatusBarMessage
{
public:
    explicit StatusBarMessage(BrowserWindow* window);

    void showMessage(const QString &message);
    void clearMessage();

private:
    BrowserWindow* m_window;
    TipLabel* m_statusBarText;
};

#endif // STATUSBARMESSAGE_H
