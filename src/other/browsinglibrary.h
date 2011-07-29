#ifndef LIBRARY_H
#define LIBRARY_H

#include <QWidget>
#include <QDesktopWidget>

namespace Ui {
    class BrowsingLibrary;
}

class HistoryManager;
class BookmarksManager;
class RSSManager;
class QupZilla;
class BrowsingLibrary : public QWidget
{
    Q_OBJECT

public:
    explicit BrowsingLibrary(QupZilla* mainClass, QWidget *parent = 0);
    ~BrowsingLibrary();

    void showHistory(QupZilla* mainClass);
    void showBookmarks(QupZilla* mainClass);
    void showRSS(QupZilla* mainClass);

    HistoryManager* historyManager() { return m_historyManager; }
    BookmarksManager* bookmarksManager() { return m_bookmarksManager; }
    RSSManager* rssManager() { return m_rssManager; }

private slots:
    void currentIndexChanged(int index);
    void search();

private:
    Ui::BrowsingLibrary *ui;
    HistoryManager* m_historyManager;
    BookmarksManager* m_bookmarksManager;
    RSSManager* m_rssManager;
    bool m_historyLoaded;
    bool m_bookmarksLoaded;
    bool m_rssLoaded;
};

#endif // LIBRARY_H
