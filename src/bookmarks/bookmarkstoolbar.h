#ifndef BOOKMARKSTOOLBAR_H
#define BOOKMARKSTOOLBAR_H

#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include <QToolBar>
#include <QMenu>
#include <QToolButton>

class QupZilla;
class BookmarksModel;

class BookmarksToolbar : public QToolBar
{
    Q_OBJECT
public:
    explicit BookmarksToolbar(QupZilla* mainClass, QWidget *parent = 0);
    void setColor(QColor color);

signals:

public slots:
    void refreshBookmarks();
    void refreshMostVisited();
    void customContextMenuRequested(const QPoint &pos);
    void hidePanel();
    void showMostVisited();

private:
    QupZilla* p_QupZilla;
    BookmarksModel* m_bookmarksModel;
    QMenu* m_menuMostVisited;

};

#endif // BOOKMARKSTOOLBAR_H
