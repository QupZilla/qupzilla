#ifndef BOOKMARKSMANAGER_H
#define BOOKMARKSMANAGER_H

#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include <QWidget>
#include <QTreeWidgetItem>
#include <QInputDialog>
#include <QPointer>

namespace Ui {
    class BookmarksManager;
}

class WebView;
class QupZilla;
class BookmarksModel;
class BookmarksManager : public QWidget
{
    Q_OBJECT

public:
    explicit BookmarksManager(QupZilla* mainClass, QWidget *parent = 0);
    ~BookmarksManager();
    void addBookmark(WebView* view);
    void insertBookmark(const QUrl &url, const QString &title);
    void setMainWindow(QupZilla* window);

public slots:
    void refreshTable();
    void insertAllTabs();

private slots:
    void deleteItem();
    void itemChanged(QTreeWidgetItem* item);
    void addFolder();
    void contextMenuRequested(const QPoint &position);
    void loadInNewTab();
    void itemControlClicked(QTreeWidgetItem* item);
    void moveBookmark();

private:
    QupZilla* getQupZilla();

    bool m_isRefreshing;
    Ui::BookmarksManager *ui;
    QPointer<QupZilla> p_QupZilla;
    BookmarksModel* m_bookmarksModel;
};

#endif // BOOKMARKSMANAGER_H
