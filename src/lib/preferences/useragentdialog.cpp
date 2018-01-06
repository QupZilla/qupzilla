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
#include "useragentdialog.h"
#include "ui_useragentdialog.h"
#include "useragentmanager.h"
#include "qztools.h"
#include "mainapplication.h"
#include "settings.h"
#include "networkmanager.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>

UserAgentDialog::UserAgentDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::UserAgentDialog)
    , m_manager(mApp->userAgentManager())
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);
    ui->globalComboBox->setLayoutDirection(Qt::LeftToRight);
    ui->table->setLayoutDirection(Qt::LeftToRight);

    QString os = QzTools::operatingSystemLong();
#ifdef Q_OS_UNIX
    if (QGuiApplication::platformName() == QL1S("xcb"))
        os.prepend(QL1S("X11; "));
    else if (QGuiApplication::platformName().startsWith(QL1S("wayland")))
        os.prepend(QL1S("Wayland; "));
#endif

    QRegularExpression chromeRx(QSL("Chrome/([^\\s]+)"));
    const QString chromeVersion = chromeRx.match(m_manager->defaultUserAgent()).captured(1);

    m_knownUserAgents << QString("Opera/9.80 (%1) Presto/2.12.388 Version/12.16").arg(os)
                      << QString("Mozilla/5.0 (%1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/%2 Safari/537.36").arg(os, chromeVersion)
                      << QString("Mozilla/5.0 (%1) AppleWebKit/602.3.12 (KHTML, like Gecko) Version/10.0.2 Safari/602.3.12").arg(os)
                      << QString("Mozilla/5.0 (%1; rv:57.0) Gecko/20100101 Firefox/57.0").arg(os);

    ui->globalComboBox->addItems(m_knownUserAgents);

    const QString globalUserAgent = m_manager->globalUserAgent();
    ui->changeGlobal->setChecked(!globalUserAgent.isEmpty());
    ui->globalComboBox->lineEdit()->setText(globalUserAgent);
    ui->globalComboBox->lineEdit()->setCursorPosition(0);

    ui->changePerSite->setChecked(m_manager->usePerDomainUserAgents());

    QHashIterator<QString, QString> i(m_manager->perDomainUserAgentsList());
    while (i.hasNext()) {
        i.next();

        QTableWidgetItem* siteItem = new QTableWidgetItem(i.key());
        QTableWidgetItem* userAgentItem = new QTableWidgetItem(i.value());

        int row = ui->table->rowCount();

        ui->table->insertRow(row);
        ui->table->setItem(row, 0, siteItem);
        ui->table->setItem(row, 1, userAgentItem);
    }

    ui->table->sortByColumn(-1);

    connect(ui->add, SIGNAL(clicked()), this, SLOT(addSite()));
    connect(ui->remove, SIGNAL(clicked()), this, SLOT(removeSite()));
    connect(ui->edit, SIGNAL(clicked()), this, SLOT(editSite()));
    connect(ui->table, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(editSite()));

    connect(ui->changeGlobal, SIGNAL(clicked(bool)), this, SLOT(enableGlobalComboBox(bool)));
    connect(ui->changePerSite, SIGNAL(clicked(bool)), this, SLOT(enablePerSiteFrame(bool)));

    enableGlobalComboBox(ui->changeGlobal->isChecked());
    enablePerSiteFrame(ui->changePerSite->isChecked());
}

void UserAgentDialog::addSite()
{
    QString site;
    QString userAgent;

    if (showEditDialog(tr("Add new site"), &site, &userAgent)) {
        QTableWidgetItem* siteItem = new QTableWidgetItem(site);
        QTableWidgetItem* userAgentItem = new QTableWidgetItem(userAgent);

        int row = ui->table->rowCount();

        ui->table->insertRow(row);
        ui->table->setItem(row, 0, siteItem);
        ui->table->setItem(row, 1, userAgentItem);
    }
}

void UserAgentDialog::removeSite()
{
    int row = ui->table->currentRow();

    QTableWidgetItem* siteItem = ui->table->item(row, 0);
    QTableWidgetItem* userAgentItem = ui->table->item(row, 1);

    if (siteItem && userAgentItem) {
        delete siteItem;
        delete userAgentItem;

        ui->table->removeRow(row);
    }
}

void UserAgentDialog::editSite()
{
    int row = ui->table->currentRow();

    QTableWidgetItem* siteItem = ui->table->item(row, 0);
    QTableWidgetItem* userAgentItem = ui->table->item(row, 1);

    if (siteItem && userAgentItem) {
        QString site = siteItem->text();
        QString userAgent = userAgentItem->text();

        if (showEditDialog(tr("Edit site"), &site, &userAgent)) {
            siteItem->setText(site);
            userAgentItem->setText(userAgent);
        }
    }
}

void UserAgentDialog::accept()
{
    QString globalUserAgent = ui->changeGlobal->isChecked() ? ui->globalComboBox->currentText() : QString();
    QStringList domainList;
    QStringList userAgentsList;

    for (int i = 0; i < ui->table->rowCount(); ++i) {
        QTableWidgetItem* siteItem = ui->table->item(i, 0);
        QTableWidgetItem* userAgentItem = ui->table->item(i, 1);

        if (!siteItem || !userAgentItem) {
            continue;
        }

        QString domain = siteItem->text().trimmed();
        QString userAgent = userAgentItem->text().trimmed();

        if (domain.isEmpty() || userAgent.isEmpty()) {
            continue;
        }

        domainList.append(domain);
        userAgentsList.append(userAgent);
    }

    Settings settings;
    settings.beginGroup("Web-Browser-Settings");
    settings.setValue("UserAgent", globalUserAgent);
    settings.endGroup();

    settings.beginGroup("User-Agent-Settings");
    settings.setValue("UsePerDomainUA", ui->changePerSite->isChecked());
    settings.setValue("DomainList", domainList);
    settings.setValue("UserAgentsList", userAgentsList);
    settings.endGroup();

    m_manager->loadSettings();
    mApp->networkManager()->loadSettings();
    close();
}

void UserAgentDialog::enableGlobalComboBox(bool enable)
{
    ui->globalComboBox->setEnabled(enable);
}

void UserAgentDialog::enablePerSiteFrame(bool enable)
{
    ui->perSiteFrame->setEnabled(enable);
}

bool UserAgentDialog::showEditDialog(const QString &title, QString* rSite, QString* rUserAgent)
{
    if (!rSite || !rUserAgent) {
        return false;
    }

    QDialog* dialog = new QDialog(this);
    QFormLayout* layout = new QFormLayout(dialog);
    QLineEdit* editSite = new QLineEdit(dialog);
    QComboBox* editAgent = new QComboBox(dialog);
    editAgent->setLayoutDirection(Qt::LeftToRight);
    editAgent->setEditable(true);
    editAgent->addItems(m_knownUserAgents);

    QDialogButtonBox* box = new QDialogButtonBox(dialog);
    box->addButton(QDialogButtonBox::Ok);
    box->addButton(QDialogButtonBox::Cancel);

    connect(box, SIGNAL(rejected()), dialog, SLOT(reject()));
    connect(box, SIGNAL(accepted()), dialog, SLOT(accept()));

    layout->addRow(new QLabel(tr("Site domain: ")), editSite);
    layout->addRow(new QLabel(tr("User Agent: ")), editAgent);
    layout->addRow(box);

    editSite->setText(*rSite);
    editAgent->lineEdit()->setText(*rUserAgent);

    editSite->setFocus();
    editAgent->lineEdit()->setCursorPosition(0);

    dialog->setWindowTitle(title);
    dialog->setMinimumSize(550, 100);
    dialog->setMaximumWidth(550);

    if (dialog->exec()) {
        *rSite = editSite->text();
        *rUserAgent = editAgent->currentText();

        return !rSite->isEmpty() && !rUserAgent->isEmpty();
    }

    return false;
}

UserAgentDialog::~UserAgentDialog()
{
    delete ui;
}
