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

    useHttpsProxyChanged(false);

    connect(ui->useHttpsProxy, SIGNAL(toggled(bool)), this, SLOT(useHttpsProxyChanged(bool)));
}

void SBI_ProxyWidget::clear()
{
    ui->proxyServer->clear();
    ui->proxyPort->clear();
    ui->proxyUsername->clear();
    ui->proxyPassword->clear();

    ui->httpsProxyServer->clear();
    ui->httpsProxyPort->clear();
    ui->httpsProxyUsername->clear();
    ui->httpsProxyPassword->clear();

    ui->proxyExceptions->clear();
    ui->pacUrl->clear();

    ui->proxyType->setCurrentIndex(0);
    ui->useHttpsProxy->setChecked(false);
    ui->noProxy->setChecked(true);
}

SBI_NetworkProxy* SBI_ProxyWidget::getProxy() const
{
    SBI_NetworkProxy* proxy = new SBI_NetworkProxy;

    proxy->setHostName(ui->proxyServer->text());
    proxy->setPort(ui->proxyPort->text().toInt());
    proxy->setUserName(ui->proxyUsername->text());
    proxy->setPassword(ui->proxyPassword->text());

    proxy->setHttpsHostName(ui->httpsProxyServer->text());
    proxy->setHttpsPort(ui->httpsProxyPort->text().toInt());
    proxy->setHttpsUserName(ui->httpsProxyUsername->text());
    proxy->setHttpsPassword(ui->httpsProxyPassword->text());

    proxy->setExceptions(ui->proxyExceptions->text().split(QLatin1Char(','), QString::SkipEmptyParts));
    proxy->setProxyAutoConfigUrl(QUrl(ui->pacUrl->text()));

    proxy->setUseDifferentProxyForHttps(ui->useHttpsProxy->isChecked());
    proxy->setType(ui->proxyType->currentIndex() == 0 ? QNetworkProxy::HttpProxy : QNetworkProxy::Socks5Proxy);

    if (ui->noProxy->isChecked()) {
        proxy->setPreference(NetworkProxyFactory::NoProxy);
    }
    else if (ui->systemProxy->isChecked()) {
        proxy->setPreference(NetworkProxyFactory::SystemProxy);
    }
    else if (ui->manualProxy->isChecked()) {
        proxy->setPreference(NetworkProxyFactory::DefinedProxy);
    }
    else if (ui->pacProxy->isChecked()) {
        proxy->setPreference(NetworkProxyFactory::ProxyAutoConfig);
    }

    return proxy;
}

void SBI_ProxyWidget::setProxy(const SBI_NetworkProxy &proxy)
{
    ui->proxyServer->setText(proxy.hostName());
    ui->proxyPort->setText(QString::number(proxy.port()));
    ui->proxyUsername->setText(proxy.userName());
    ui->proxyPassword->setText(proxy.password());

    ui->httpsProxyServer->setText(proxy.httpsHostName());
    ui->httpsProxyPort->setText(QString::number(proxy.httpsPort()));
    ui->httpsProxyUsername->setText(proxy.httpsUserName());
    ui->httpsProxyPassword->setText(proxy.httpsPassword());

    ui->useHttpsProxy->setChecked(proxy.useDifferentProxyForHttps());
    ui->proxyExceptions->setText(proxy.exceptions().join(QLatin1String(",")));
    ui->proxyType->setCurrentIndex(proxy.type() == QNetworkProxy::HttpProxy ? 0 : 1);

    switch (proxy.preference()) {
    case NetworkProxyFactory::NoProxy:
        ui->noProxy->setChecked(true);
        break;

    case NetworkProxyFactory::SystemProxy:
        ui->systemProxy->setChecked(true);
        break;

    case NetworkProxyFactory::DefinedProxy:
        ui->manualProxy->setChecked(true);
        break;

    case NetworkProxyFactory::ProxyAutoConfig:
        ui->pacProxy->setChecked(true);
        break;

    default:
        break;
    }
}

void SBI_ProxyWidget::useHttpsProxyChanged(bool enable)
{
    ui->httpsCredentialsLayout_2->setEnabled(enable);
    ui->httpsServerLayout_2->setEnabled(enable);
}

SBI_ProxyWidget::~SBI_ProxyWidget()
{
    delete ui;
}
