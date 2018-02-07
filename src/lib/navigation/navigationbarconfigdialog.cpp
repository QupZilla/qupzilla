/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2018 David Rosca <nowrep@gmail.com>
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
#include "navigationbarconfigdialog.h"
#include "ui_navigationbarconfigdialog.h"
#include "navigationbar.h"
#include "toolbutton.h"
#include "settings.h"
#include "mainapplication.h"
#include "websearchbar.h"

NavigationBarConfigDialog::NavigationBarConfigDialog(NavigationBar *navigationBar)
    : QDialog(navigationBar)
    , ui(new Ui::NavigationBarConfigDialog)
    , m_navigationBar(navigationBar)
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);

    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &NavigationBarConfigDialog::buttonClicked);

    loadSettings();
}

void NavigationBarConfigDialog::loadSettings()
{
    auto createItem = [this](const NavigationBar::WidgetData &data) {
        QListWidgetItem *item = new QListWidgetItem();
        item->setText(data.name);
        item->setData(Qt::UserRole + 10, data.id);
#if 0
        // XXX: Crashes in Qt on items drag&drop...
        ToolButton *button = qobject_cast<ToolButton*>(data.widget);
        if (button) {
            item->setIcon(button->icon());
        }
#endif
        return item;
    };

    ui->currentItems->clear();
    for (const QString &id : qAsConst(m_navigationBar->m_layoutIds)) {
        NavigationBar::WidgetData data = m_navigationBar->m_widgets.value(id);
        if (data.id.isEmpty()) {
            data.id = id;
            data.name = id;
        }
        ui->currentItems->addItem(createItem(data));
    }

    ui->availableItems->clear();
    for (const NavigationBar::WidgetData &data : qAsConst(m_navigationBar->m_widgets)) {
        if (!m_navigationBar->m_layoutIds.contains(data.id)) {
            ui->availableItems->addItem(createItem(data));
        }
    }

    ui->showSearchBar->setChecked(m_navigationBar->webSearchBar()->isVisible());
}

void NavigationBarConfigDialog::saveSettings()
{
    QStringList ids;
    for (int i = 0; i < ui->currentItems->count(); ++i) {
        ids.append(ui->currentItems->item(i)->data(Qt::UserRole + 10).toString());
    }

    Settings settings;
    settings.beginGroup(QSL("NavigationBar"));
    settings.setValue(QSL("Layout"), ids);
    settings.setValue(QSL("ShowSearchBar"), ui->showSearchBar->isChecked());
    settings.endGroup();

    const auto windows = mApp->windows();
    for (BrowserWindow *window : windows) {
        window->navigationBar()->loadSettings();
    }
}

void NavigationBarConfigDialog::resetToDefaults()
{
    Settings settings;
    settings.beginGroup(QSL("NavigationBar"));
    settings.remove(QSL("Layout"));
    settings.remove(QSL("ShowSearchBar"));
    settings.endGroup();

    const auto windows = mApp->windows();
    for (BrowserWindow *window : windows) {
        window->navigationBar()->loadSettings();
    }
}

void NavigationBarConfigDialog::buttonClicked(QAbstractButton *button)
{
    switch (ui->buttonBox->buttonRole(button)) {
    case QDialogButtonBox::ApplyRole:
        saveSettings();
        loadSettings();
        break;

    case QDialogButtonBox::RejectRole:
        close();
        break;

    case QDialogButtonBox::ResetRole:
        resetToDefaults();
        loadSettings();
        break;

    case QDialogButtonBox::AcceptRole:
        saveSettings();
        close();
        break;

    default:
        break;
    }
}
