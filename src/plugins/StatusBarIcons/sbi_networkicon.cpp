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
#include "sbi_networkicon.h"
#include "mainapplication.h"
#include "networkmanager.h"
#include "networkproxyfactory.h"
#include "qupzilla.h"

#include <QDebug>

SBI_NetworkIcon::SBI_NetworkIcon(QupZilla* window, const QString &settingsPath)
    : ClickableLabel(window)
    , p_QupZilla(window)
    , m_settingsFile(settingsPath + "networkicon.ini")
{
    setCursor(Qt::PointingHandCursor);

    connect(mApp->networkManager(), SIGNAL(networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)), this, SLOT(networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)));

    networkAccessibleChanged(mApp->networkManager()->networkAccessible());
}

void SBI_NetworkIcon::networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessibility)
{
    switch (accessibility) {
    case QNetworkAccessManager::Accessible:
        setPixmap(QIcon(":sbi/data/network-online.png").pixmap(16));
        break;

    case QNetworkAccessManager::NotAccessible:
        setPixmap(QIcon(":sbi/data/network-offline.png").pixmap(16));
        break;

    default:
        setPixmap(QIcon(":sbi/data/network-unknown.png").pixmap(16));
        break;
    }

    updateToolTip();
}

void SBI_NetworkIcon::updateToolTip()
{
    QString tooltip = tr("Shows network status and manages proxy<br/><br/><b>Network:</b><br/>%1<br/><br/><b>Proxy:</b><br/>%2");

    switch (mApp->networkManager()->networkAccessible()) {
    case QNetworkAccessManager::Accessible:
        tooltip = tooltip.arg(tr("Connected"));
        break;

    case QNetworkAccessManager::NotAccessible:
        tooltip = tooltip.arg(tr("Offline"));
        break;

    default:
        tooltip = tooltip.arg(tr("Unknown"));
        break;
    }

    switch (mApp->networkManager()->proxyFactory()->proxyPreference()) {
    case NetworkProxyFactory::SystemProxy:
        tooltip = tooltip.arg("System proxy");
        break;

    case NetworkProxyFactory::NoProxy:
        tooltip = tooltip.arg("No proxy");
        break;

    case NetworkProxyFactory::ProxyAutoConfig:
        tooltip = tooltip.arg("PAC (Proxy Auto-Config)");
        break;

    case NetworkProxyFactory::DefinedProxy:
        tooltip = tooltip.arg("User defined");
        break;

    default:
        qWarning() << "Unknown NetworkProxyFactory::ProxyPreference!";
        break;
    }

    setToolTip(tooltip);
}

void SBI_NetworkIcon::enterEvent(QEvent* event)
{
    updateToolTip();

    ClickableLabel::enterEvent(event);
}
