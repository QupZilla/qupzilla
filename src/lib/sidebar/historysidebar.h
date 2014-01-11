/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
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
#ifndef HISTORYSIDEBAR_H
#define HISTORYSIDEBAR_H

#include <QWidget>

#include "qz_namespace.h"
#include "historyview.h"

namespace Ui
{
class HistorySideBar;
}

class QupZilla;

class QT_QUPZILLA_EXPORT HistorySideBar : public QWidget
{
    Q_OBJECT

public:
    explicit HistorySideBar(QupZilla* mainClass, QWidget* parent = 0);
    ~HistorySideBar();

private slots:
    void openLink(const QUrl &url, HistoryView::OpenBehavior openIn);

private:
    Ui::HistorySideBar* ui;
    QupZilla* p_QupZilla;
};

#endif // HISTORYSIDEBAR_H
