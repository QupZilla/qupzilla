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
    setIcon(QIcon(QSL(":icons/other/adblock.png")));

    updateState();

    connect(this, &AbstractButtonInterface::clicked, this, &AdBlockIcon::clicked);
    connect(this, &AbstractButtonInterface::webPageChanged, this, &AdBlockIcon::webPageChanged);
    connect(AdBlockManager::instance(), &AdBlockManager::enabledChanged, this, &AdBlockIcon::updateState);
}

AdBlockIcon::~AdBlockIcon()
{
    for (int i = 0; i < m_blockedPopups.count(); ++i)
        delete m_blockedPopups.at(i).first;
}

QString AdBlockIcon::id() const
{
    return QSL("adblock-icon");
}

QString AdBlockIcon::name() const
{
    return tr("AdBlock Icon");
}

void AdBlockIcon::popupBlocked(const QString &ruleString, const QUrl &url)
{
    int index = ruleString.lastIndexOf(QLatin1String(" ("));

    const QString subscriptionName = ruleString.left(index);
    const QString filter = ruleString.mid(index + 2, ruleString.size() - index - 3);
    AdBlockSubscription* subscription = AdBlockManager::instance()->subscriptionByName(subscriptionName);
    if (filter.isEmpty() || !subscription) {
        return;
    }

    QPair<AdBlockRule*, QUrl> pair;
    pair.first = new AdBlockRule(filter, subscription);
    pair.second = url;
    m_blockedPopups.append(pair);

    mApp->desktopNotifications()->showNotification(QPixmap(":html/adblock_big.png"), tr("Blocked popup window"), tr("AdBlock blocked unwanted popup window."));
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
        return;
    }
    if (!AdBlockManager::instance()->isEnabled()) {
        setActive(false);
        setToolTip(tr("AdBlock is disabled"));
        return;
    }
    if (!AdBlockManager::instance()->canRunOnScheme(page->url().scheme())) {
        setActive(false);
        setToolTip(tr("AdBlock is disabled on this site "));
        return;
    }
    setActive(true);
    setToolTip(tr("AdBlock is active"));
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

    if (!m_blockedPopups.isEmpty()) {
        menu.addAction(tr("Blocked Popup Windows"))->setEnabled(false);
        for (int i = 0; i < m_blockedPopups.count(); i++) {
            const QPair<AdBlockRule*, QUrl> &pair = m_blockedPopups.at(i);

            QString address = pair.second.toString().right(55);
            QString actionText = tr("%1 with (%2)").arg(address, pair.first->filter()).replace(QLatin1Char('&'), QLatin1String("&&"));

            QAction* action = menu.addAction(actionText, manager, SLOT(showRule()));
            action->setData(QVariant::fromValue((void*)pair.first));
        }
    }

    menu.exec(controller->popupPosition(menu.sizeHint()));
}
