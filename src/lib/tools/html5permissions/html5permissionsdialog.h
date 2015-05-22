/* ============================================================
* QupZilla - WebKit based browser
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
#ifndef HTML5PERMISSIONSDIALOG_H
#define HTML5PERMISSIONSDIALOG_H

#include <QDialog>
#include <QStringList>
#include <QWebEnginePage>

namespace Ui
{
class HTML5PermissionsDialog;
}

class HTML5PermissionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HTML5PermissionsDialog(QWidget* parent = 0);
    ~HTML5PermissionsDialog();

    void showFeaturePermissions(QWebEnginePage::Feature feature);

private slots:
    void removeEntry();
    void featureIndexChanged();

    void saveSettings();

private:
    enum Role { Allow, Deny };

    void loadSettings();
    QWebEnginePage::Feature currentFeature() const;

    Ui::HTML5PermissionsDialog* ui;

    QHash<QWebEnginePage::Feature, QStringList> m_granted;
    QHash<QWebEnginePage::Feature, QStringList> m_denied;
};

#endif // HTML5PERMISSIONSDIALOG_H
