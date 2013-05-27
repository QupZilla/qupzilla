/* ============================================================
* StatusBarIcons - Extra icons in statusbar for QupZilla
* Copyright (C) 2013  David Rosca <nowrep@gmail.com>
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

#include "clickablelabel.h"

class QupZilla;

class SBI_NetworkIcon : public ClickableLabel
{
    Q_OBJECT

public:
    explicit SBI_NetworkIcon(QupZilla* window, const QString &settingsPath);

private slots:
    void networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessibility);

private:
    void updateToolTip();

    void enterEvent(QEvent* event);

    QupZilla* p_QupZilla;
    QString m_settingsFile;

    QIcon m_icon;
};

#endif // SBI_NETWORKICON_H
