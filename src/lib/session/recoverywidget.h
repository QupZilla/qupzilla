/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012 Franz Fellner <alpine.art.de@googlemail.com>
*                         David Rosca <nowrep@gmail.com>
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
#ifndef RECOVERYWIDGET_H
#define RECOVERYWIDGET_H

#include <QWidget>

namespace Ui
{
class RecoveryWidget;
}

class QTreeWidgetItem;

class WebView;
class QupZilla;

class RecoveryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RecoveryWidget(WebView* view, QupZilla* mainClass);
    ~RecoveryWidget();

private slots:
    void onItemChanged(QTreeWidgetItem* item, int column);

    void restoreSession();
    void newSession();

private:
    Ui::RecoveryWidget* ui;

    WebView* m_view;
    QupZilla* p_QupZilla;
};

#endif // RECOVERYWIDGET_H
