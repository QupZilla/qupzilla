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
#include "webview.h"
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
    connect(this, &AbstractButtonInterface::webViewChanged, this, &AdBlockIcon::webViewChanged);
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
    WebView *view = webView();
    if (!view) {
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
    if (!AdBlockManager::instance()->canRunOnScheme(view->url().scheme())) {
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
    WebView *view = webView();
    if (!view) {
        return;
    }
    const int count = AdBlockManager::instance()->blockedRequestsForUrl(view->url()).count();
    if (count > 0) {
        setBadgeText(QString::number(count));
    } else {
        setBadgeText(QString());
    }
}

void AdBlockIcon::webViewChanged(WebView *view)
{
    updateState();

    if (m_view) {
        disconnect(m_view.data(), &WebView::urlChanged, this, &AdBlockIcon::updateState);
    }

    m_view = view;

    if (m_view) {
        connect(m_view.data(), &WebView::urlChanged, this, &AdBlockIcon::updateState);
    }
}

void AdBlockIcon::clicked(ClickController *controller)
{
    WebView *view = webView();
    if (!view) {
        return;
    }

    AdBlockManager* manager = AdBlockManager::instance();
    AdBlockCustomList* customList = manager->customList();

    const QUrl pageUrl = view->url();

    QMenu *menu = new QMenu();
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->addAction(tr("Show AdBlock &Settings"), manager, SLOT(showDialog()));
    menu->addSeparator();

    if (!pageUrl.host().isEmpty() && manager->isEnabled() && manager->canRunOnScheme(pageUrl.scheme())) {
        const QString host = view->url().host().contains(QLatin1String("www.")) ? pageUrl.host().mid(4) : pageUrl.host();
        const QString hostFilter = QString("@@||%1^$document").arg(host);
        const QString pageFilter = QString("@@|%1|$document").arg(pageUrl.toString());

        QAction* act = menu->addAction(tr("Disable on %1").arg(host));
        act->setCheckable(true);
        act->setChecked(customList->containsFilter(hostFilter));
        act->setData(hostFilter);
        connect(act, SIGNAL(triggered()), this, SLOT(toggleCustomFilter()));

        act = menu->addAction(tr("Disable only on this page"));
        act->setCheckable(true);
        act->setChecked(customList->containsFilter(pageFilter));
        act->setData(pageFilter);
        connect(act, SIGNAL(triggered()), this, SLOT(toggleCustomFilter()));
    }

    connect(menu, &QMenu::aboutToHide, this, [=]() {
        controller->popupClosed();
    });

    menu->popup(controller->popupPosition(menu->sizeHint()));
}

void AdBlockIcon::blockedRequestsChanged(const QUrl &url)
{
    WebView *view = webView();
    if (!view || url != view->url()) {
        return;
    }
    updateState();
}
