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
#ifndef USERAGENTDIALOG_H
#define USERAGENTDIALOG_H

#include <QDialog>
#include <QStringList>

#include "qzcommon.h"

class UserAgentManager;

namespace Ui
{
class UserAgentDialog;
}

class QUPZILLA_EXPORT UserAgentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UserAgentDialog(QWidget* parent = 0);
    ~UserAgentDialog();

private slots:
    void addSite();
    void removeSite();
    void editSite();

    void accept();

    void enableGlobalComboBox(bool enable);
    void enablePerSiteFrame(bool enable);

private:
    bool showEditDialog(const QString &title, QString* rSite, QString* rUserAgent);

    Ui::UserAgentDialog* ui;
    UserAgentManager* m_manager;

    QStringList m_knownUserAgents;
};

#endif // USERAGENTDIALOG_H
