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
#include "adblockicon.h"
#include "adblockrule.h"
#include "adblockmanager.h"
#include "adblocksubscription.h"
#include "mainapplication.h"
#include "browserwindow.h"
#include "webpage.h"
#include "tabbedwebview.h"
#include "tabwidget.h"
#include "desktopnotificationsfactory.h"
#include "qztools.h"

#include <QMenu>

AdBlockIcon::AdBlockIcon(QObject *parent)
    : AbstractButtonInterface(parent)
{
    setTitle(tr("AdBlock"));
    setIcon(QIcon(QSL(":adblock/data/adblock.png")));

    updateState();

    connect(this, &AbstractButtonInterface::clicked, this, &AdBlockIcon::clicked);
    connect(this, &AbstractButtonInterface::webPageChanged, this, &AdBlockIcon::webPageChanged);
    connect(AdBlockManager::instance(), &AdBlockManager::enabledChanged, this, &AdBlockIcon::updateState);
    connect(AdBlockManager::instance(), &AdBlockManager::blockedRequestsChanged, this, &AdBlockIcon::blockedRequestsChanged);
}

QString AdBlockIcon::id() const
{
    return QSL("adblock-icon");
}

QString AdBlockIcon::name() const
{
    return tr("AdBlock Icon");
}

void AdBlockIcon::toggleCustomFilter()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (!action) {
        return;
    }

    const QString filter = action->data().toString();
    AdBlockManager* manager = AdBlockManager::instance();
    AdBlockCustomList* customList = manager->customList();

    if (customList->containsFilter(filter)) {
        customList->removeFilter(filter);
    }
    else {
        AdBlockRule* rule = new AdBlockRule(filter, customList);
        customList->addRule(rule);
    }
}

void AdBlockIcon::updateState()
{
    WebPage *page = webPage();
    if (!page) {
        setActive(false);
        setToolTip(name());
        setBadgeText(QString());
        return;
    }
    if (!AdBlockManager::instance()->isEnabled()) {
        setActive(false);
        setToolTip(tr("AdBlock is disabled"));
        setBadgeText(QString());
        return;
    }
    if (!AdBlockManager::instance()->canRunOnScheme(page->url().scheme())) {
        setActive(false);
        setToolTip(tr("AdBlock is disabled on this site "));
        setBadgeText(QString());
        return;
    }

    setActive(true);
    setToolTip(tr("AdBlock is active"));
    updateBadgeText();
}

void AdBlockIcon::updateBadgeText()
{
    WebPage *page = webPage();
    if (!page) {
        return;
    }
    const int count = AdBlockManager::instance()->blockedRequestsForUrl(page->url()).count();
    if (count > 0) {
        setBadgeText(QString::number(count));
    } else {
        setBadgeText(QString());
    }
}

void AdBlockIcon::webPageChanged(WebPage *page)
{
    updateState();

    if (m_page) {
        disconnect(m_page.data(), &QWebEnginePage::urlChanged, this, &AdBlockIcon::updateState);
    }

    m_page = page;

    if (m_page) {
        connect(m_page.data(), &QWebEnginePage::urlChanged, this, &AdBlockIcon::updateState);
    }
}

void AdBlockIcon::clicked(ClickController *controller)
{
    WebPage *page = webPage();
    if (!page) {
        return;
    }

    AdBlockManager* manager = AdBlockManager::instance();
    AdBlockCustomList* customList = manager->customList();

    const QUrl pageUrl = page->url();

    QMenu menu;
    menu.addAction(tr("Show AdBlock &Settings"), manager, SLOT(showDialog()));
    menu.addSeparator();

    if (!pageUrl.host().isEmpty() && manager->isEnabled() && manager->canRunOnScheme(pageUrl.scheme())) {
        const QString host = page->url().host().contains(QLatin1String("www.")) ? pageUrl.host().mid(4) : pageUrl.host();
        const QString hostFilter = QString("@@||%1^$document").arg(host);
        const QString pageFilter = QString("@@|%1|$document").arg(pageUrl.toString());

        QAction* act = menu.addAction(tr("Disable on %1").arg(host));
        act->setCheckable(true);
        act->setChecked(customList->containsFilter(hostFilter));
        act->setData(hostFilter);
        connect(act, SIGNAL(triggered()), this, SLOT(toggleCustomFilter()));

        act = menu.addAction(tr("Disable only on this page"));
        act->setCheckable(true);
        act->setChecked(customList->containsFilter(pageFilter));
        act->setData(pageFilter);
        connect(act, SIGNAL(triggered()), this, SLOT(toggleCustomFilter()));

        menu.addSeparator();
    }

    menu.exec(controller->popupPosition(menu.sizeHint()));
}

void AdBlockIcon::blockedRequestsChanged(const QUrl &url)
{
    WebPage *page = webPage();
    if (!page || url != page->url()) {
        return;
    }
    updateState();
}
