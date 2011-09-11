#ifndef BOOKMARKICON_H
#define BOOKMARKICON_H

#include <QUrl>

#include "clickablelabel.h"
#include "bookmarksmodel.h"
class QupZilla;
class BookmarksModel;
class BookmarkIcon : public ClickableLabel
{
    Q_OBJECT
public:
    explicit BookmarkIcon(QupZilla* mainClass, QWidget* parent = 0);
    void checkBookmark(const QUrl &url);

signals:

public slots:

private slots:
    void iconClicked();
    void bookmarkAdded(const BookmarksModel::Bookmark &bookmark);
    void bookmarkDeleted(const BookmarksModel::Bookmark &bookmark);

private:
    void setBookmarkSaved();
    void setBookmarkDisabled();

    QupZilla* p_QupZilla;
    BookmarksModel* m_bookmarksModel;

    QUrl m_lastUrl;

};

#endif // BOOKMARKICON_H
