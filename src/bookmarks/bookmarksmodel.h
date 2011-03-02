#ifndef BOOKMARKSMODEL_H
#define BOOKMARKSMODEL_H
#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include <QUrl>
#include <QSettings>
#include <QSqlQuery>

class WebView;
class BookmarksModel
{
public:
    explicit BookmarksModel();
    void loadSettings();
    inline bool isShowingMostVisited() { return m_showMostVisited; }
    void setShowingMostVisited(bool state);

    bool isBookmarked(QUrl url);
    int bookmarkId(QUrl url);
    int bookmarkId(QUrl url, QString title, QString folder);
    QStringList getBookmark(int id);

    bool saveBookmark(QUrl url, QString title, QString folder = "unsorted");
    bool saveBookmark(WebView* view, QString folder = "unsorted");

    bool removeBookmark(int id);
    bool removeBookmark(QUrl url);
    bool removeBookmark(WebView* view);

    bool editBookmark(int id, QString title, QString folder);
    bool editBookmark(int id, QUrl url, QString title);

signals:

public slots:

private:
    bool m_showMostVisited;

};

#endif // BOOKMARKSMODEL_H
