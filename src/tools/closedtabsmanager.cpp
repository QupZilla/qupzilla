#include "closedtabsmanager.h"
#include "webview.h"
#include "qwebhistory.h"

ClosedTabsManager::ClosedTabsManager(QObject *parent) :
    QObject(parent)
{
}

void ClosedTabsManager::saveView(WebView* view)
{
    if (view->url().isEmpty() && view->history()->items().count() == 0)
        return;

    Tab tab;
    tab.url = view->url();
    tab.title = view->title();
    QDataStream tabHistoryStream(&tab.history, QIODevice::WriteOnly);
    tabHistoryStream << view->history();

    m_closedTabs.prepend(tab);
}

ClosedTabsManager::Tab ClosedTabsManager::getFirstClosedTab()
{
    Tab tab;
    if (m_closedTabs.count() > 0) {
        tab = m_closedTabs.takeFirst();
        m_closedTabs.removeOne(tab);
    }

    return tab;
}

ClosedTabsManager::Tab ClosedTabsManager::getTabAt(int index)
{
    Tab tab;
    if (m_closedTabs.count() > 0) {
        tab = m_closedTabs.at(index);
        m_closedTabs.removeOne(tab);
    }

    return tab;
}

bool ClosedTabsManager::isClosedTabAvailable()
{
    return (m_closedTabs.count() != 0);
}
