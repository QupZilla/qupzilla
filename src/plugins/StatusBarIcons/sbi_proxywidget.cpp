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
#include "sbi_proxywidget.h"
#include "sbi_networkproxy.h"
#include "ui_sbi_proxywidget.h"

SBI_ProxyWidget::SBI_ProxyWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::SBI_ProxyWidget)
{
    ui->setupUi(this);
}

void SBI_ProxyWidget::clear()
{
    ui->proxyServer->clear();
    ui->proxyPort->clear();
    ui->proxyUsername->clear();
    ui->proxyPassword->clear();

    ui->proxyType->setCurrentIndex(0);
    ui->noProxy->setChecked(true);
}

SBI_NetworkProxy* SBI_ProxyWidget::getProxy() const
{
    SBI_NetworkProxy* proxy = new SBI_NetworkProxy;

    proxy->setHostName(ui->proxyServer->text());
    proxy->setPort(ui->proxyPort->text().toInt());
    proxy->setUserName(ui->proxyUsername->text());
    proxy->setPassword(ui->proxyPassword->text());

    if (ui->noProxy->isChecked()) {
        proxy->setType(QNetworkProxy::NoProxy);
    } else if (ui->systemProxy->isChecked()) {
        proxy->setType(QNetworkProxy::DefaultProxy);
    } else {
        proxy->setType(ui->proxyType->currentIndex() == 0 ? QNetworkProxy::HttpProxy : QNetworkProxy::Socks5Proxy);
    }

    return proxy;
}

void SBI_ProxyWidget::setProxy(const SBI_NetworkProxy &proxy)
{
    ui->proxyServer->setText(proxy.hostName());
    ui->proxyPort->setText(QString::number(proxy.port()));
    ui->proxyUsername->setText(proxy.userName());
    ui->proxyPassword->setText(proxy.password());
    ui->proxyType->setCurrentIndex(0);

    switch (proxy.type()) {
    case QNetworkProxy::NoProxy:
        ui->noProxy->setChecked(true);
        break;

    case QNetworkProxy::DefaultProxy:
        ui->systemProxy->setChecked(true);
        break;

    case QNetworkProxy::HttpProxy:
        ui->manualProxy->setChecked(true);
        ui->proxyType->setCurrentIndex(0);
        break;

    case QNetworkProxy::Socks5Proxy:
        ui->manualProxy->setChecked(true);
        ui->proxyType->setCurrentIndex(1);
        break;

    default:
        break;
    }
}

SBI_ProxyWidget::~SBI_ProxyWidget()
{
    delete ui;
}
