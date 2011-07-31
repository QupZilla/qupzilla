#include "webhistoryinterface.h"
#include "mainapplication.h"
#include "historymodel.h"

WebHistoryInterface::WebHistoryInterface(QObject *parent) :
    QWebHistoryInterface(parent)
{
}

void WebHistoryInterface::addHistoryEntry(const QString &url)
{
    m_clickedLinks.append(url);
}

bool WebHistoryInterface::historyContains(const QString &url) const
{
    return m_clickedLinks.contains(url);
}
