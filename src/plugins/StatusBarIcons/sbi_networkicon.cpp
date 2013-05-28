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
#include "sbi_networkicondialog.h"
#include "sbi_networkproxy.h"
#include "sbi_networkmanager.h"
#include "mainapplication.h"
#include "networkmanager.h"
#include "networkproxyfactory.h"
#include "qupzilla.h"

#include <QDebug>
#include <QMenu>

SBI_NetworkIcon::SBI_NetworkIcon(QupZilla* window)
    : ClickableLabel(window)
    , p_QupZilla(window)
{
    setCursor(Qt::PointingHandCursor);

    networkAccessibleChanged(mApp->networkManager()->networkAccessible());

    connect(mApp->networkManager(), SIGNAL(networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)), this, SLOT(networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)));
    connect(this, SIGNAL(clicked(QPoint)), this, SLOT(showMenu(QPoint)));
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

void SBI_NetworkIcon::showDialog()
{
    SBI_NetworkIconDialog dialog(p_QupZilla);
    dialog.exec();
}

void SBI_NetworkIcon::showMenu(const QPoint &pos)
{
    QFont boldFont = font();
    boldFont.setBold(true);

    QMenu menu;
    menu.addAction(QIcon::fromTheme("preferences-system-network", QIcon(":sbi/data/preferences-network.png")), tr("Proxy configuration"))->setFont(boldFont);

    QMenu* proxyMenu = menu.addMenu(tr("Select proxy"));

    const QHash<QString, SBI_NetworkProxy*> &proxies = SBINetManager->proxies();

    QHashIterator<QString, SBI_NetworkProxy*> it(proxies);
    while (it.hasNext()) {
        it.next();
        QAction* act = proxyMenu->addAction(it.key(), this, SLOT(useProxy()));
        act->setData(it.key());
        act->setCheckable(true);
        act->setChecked(it.value() == SBINetManager->currentProxy());
    }

    if (proxyMenu->actions().count() == 0) {
        proxyMenu->addAction(tr("Empty"))->setEnabled(false);
    }

    menu.addSeparator();
    menu.addAction(tr("Manage proxies"), this, SLOT(showDialog()));
    menu.exec(pos);
}

void SBI_NetworkIcon::useProxy()
{
    if (QAction* act = qobject_cast<QAction*>(sender())) {
        SBINetManager->setCurrentProxy(act->data().toString());
    }
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

    if (SBINetManager->currentProxy()) {
        tooltip.append(QString(" (%1)").arg(SBINetManager->currentProxyName()));
    }

    setToolTip(tooltip);
}

void SBI_NetworkIcon::enterEvent(QEvent* event)
{
    updateToolTip();

    ClickableLabel::enterEvent(event);
}
