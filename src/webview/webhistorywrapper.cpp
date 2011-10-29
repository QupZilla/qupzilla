#include "webhistorywrapper.h"

WebHistoryWrapper::WebHistoryWrapper(QObject* parent)
    : QObject(parent)
{
}

QList<QWebHistoryItem> WebHistoryWrapper::forwardItems(int maxItems, QWebHistory* history)
{
    QList<QWebHistoryItem> list;
    QUrl lastUrl = history->currentItem().url();

    int count = 0;
    foreach (QWebHistoryItem item, history->forwardItems(maxItems + 5)) {
        if (item.url() == lastUrl || count == maxItems)
            continue;

        lastUrl = item.url();
        list.append(item);
        count++;
    }

    return list;
}

QList<QWebHistoryItem> WebHistoryWrapper::backItems(int maxItems, QWebHistory* history)
{
    QList<QWebHistoryItem> list;
    QUrl lastUrl = history->currentItem().url();

    int count = 0;
    QList<QWebHistoryItem> bItems = history->backItems(maxItems + 5);
    for (int i = bItems.count() - 1; i >= 0; i--) {
        QWebHistoryItem item = bItems.at(i);
        if (item.url() == lastUrl || count == maxItems)
            continue;

        lastUrl = item.url();
        list.append(item);
        count++;
    }

    return list;
}

bool WebHistoryWrapper::canGoForward(QWebHistory* history)
{
    return !forwardItems(1, history).isEmpty();
}

bool WebHistoryWrapper::canGoBack(QWebHistory* history)
{
    return !backItems(1, history).isEmpty();
}

void WebHistoryWrapper::goBack(QWebHistory* history)
{
    QList<QWebHistoryItem> items = backItems(1, history);

    if (items.isEmpty())
        return;

    history->goToItem(items.at(0));
}

void WebHistoryWrapper::goForward(QWebHistory* history)
{
    QList<QWebHistoryItem> items = forwardItems(1, history);

    if (items.isEmpty())
        return;

    history->goToItem(items.at(0));
}
