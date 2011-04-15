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
    inline void setBookmarkSaved()
    {
        setPixmap(QPixmap(":/icons/locationbar/star.png"));
        setToolTip(tr("Edit this bookmark"));
    }

    inline void setBookmarkDisabled()
    {
        setPixmap(QPixmap(":/icons/locationbar/starg.png"));
        setToolTip(tr("Bookmark this Page"));
    }
    QupZilla* p_QupZilla;
    BookmarksModel* m_bookmarksModel;

    QUrl m_lastUrl;

};

#endif // BOOKMARKICON_H
