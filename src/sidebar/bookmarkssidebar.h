#ifndef BOOKMARKSSIDEBAR_H
#define BOOKMARKSSIDEBAR_H

#include <QWidget>
#include <QTreeWidgetItem>
#include <QInputDialog>
#include <QPointer>

#include "bookmarksmodel.h"

namespace Ui {
    class BookmarksSideBar;
}

class WebView;
class QupZilla;
class BookmarksModel;
class BookmarksSideBar : public QWidget
{
    Q_OBJECT

public:
    explicit BookmarksSideBar(QupZilla* mainClass, QWidget* parent = 0);
    ~BookmarksSideBar();
    void setMainWindow(QupZilla* window);

public slots:
    void refreshTable();

private slots:
    void deleteItem();
    void contextMenuRequested(const QPoint &position);
    void loadInNewTab();
    void itemControlClicked(QTreeWidgetItem* item);
    void itemDoubleClicked(QTreeWidgetItem* item);
    void moveBookmark();

    void addBookmark(const BookmarksModel::Bookmark &bookmark);
    void removeBookmark(const BookmarksModel::Bookmark &bookmark);
    void bookmarkEdited(const BookmarksModel::Bookmark &before, const BookmarksModel::Bookmark &after);
    void addFolder(const QString &name);
    void removeFolder(const QString &name);

private:
    QupZilla* getQupZilla();

    bool m_isRefreshing;
    Ui::BookmarksSideBar* ui;
    QPointer<QupZilla> p_QupZilla;
    BookmarksModel* m_bookmarksModel;
};

#endif // BOOKMARKSSIDEBAR_H
