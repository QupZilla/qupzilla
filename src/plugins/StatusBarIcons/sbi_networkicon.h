/* ============================================================
* StatusBarIcons - Extra icons in statusbar for QupZilla
* Copyright (C) 2013-2014  David Rosca <nowrep@gmail.com>
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
#ifndef SBI_NETWORKICON_H
#define SBI_NETWORKICON_H

#include <QNetworkAccessManager>

#include "sbi_icon.h"

class QNetworkConfigurationManager;

class SBI_NetworkIcon : public SBI_Icon
{
    Q_OBJECT

public:
    explicit SBI_NetworkIcon(BrowserWindow* window);

private slots:
    void onlineStateChanged(bool online);
    void showDialog();

    void showMenu(const QPoint &pos);
    void useProxy();

private:
    void updateToolTip();
    void enterEvent(QEvent* event);

    QNetworkConfigurationManager* m_networkConfiguration;
};

#endif // SBI_NETWORKICON_H
