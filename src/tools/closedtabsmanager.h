#ifndef CLOSEDTABSMANAGER_H
#define CLOSEDTABSMANAGER_H

#include <QObject>
#include <QUrl>

class WebView;
class ClosedTabsManager : public QObject
{
    Q_OBJECT
public:
    explicit ClosedTabsManager(QObject* parent = 0);
    struct Tab {
        QUrl url;
        QByteArray history;
        QString title;

        bool operator==(const Tab &a)
        {
            return (a.url == url) && (a.history == history);
        }
    };

    void saveView(WebView* view);
    ClosedTabsManager::Tab getFirstClosedTab();
    ClosedTabsManager::Tab getTabAt(int index);

    bool isClosedTabAvailable();

    QList<ClosedTabsManager::Tab> allClosedTabs() { return m_closedTabs; }

signals:

public slots:

private:
    QList<ClosedTabsManager::Tab> m_closedTabs;

};

#endif // CLOSEDTABSMANAGER_H
