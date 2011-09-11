#include "bookmarkicon.h"
#include "mainapplication.h"
#include "qupzilla.h"
#include "bookmarksmodel.h"
#include "bookmarkswidget.h"

BookmarkIcon::BookmarkIcon(QupZilla* mainClass, QWidget* parent)
    : ClickableLabel(parent)
    , p_QupZilla(mainClass)
    , m_bookmarksModel(0)
{
    setObjectName("locationbar-bookmarkicon");
    setCursor(Qt::PointingHandCursor);
    setToolTip(tr("Bookmark this Page"));
    setFocusPolicy(Qt::ClickFocus);

    m_bookmarksModel = mApp->bookmarksModel();
    connect(this, SIGNAL(clicked(QPoint)), this, SLOT(iconClicked()));
    connect(m_bookmarksModel, SIGNAL(bookmarkAdded(BookmarksModel::Bookmark)), this, SLOT(bookmarkAdded(BookmarksModel::Bookmark)));
    connect(m_bookmarksModel, SIGNAL(bookmarkDeleted(BookmarksModel::Bookmark)), this, SLOT(bookmarkDeleted(BookmarksModel::Bookmark)));
}

void BookmarkIcon::iconClicked()
{
    QUrl url = p_QupZilla->weView()->url();

    if (m_bookmarksModel->isBookmarked(url)) {
        BookmarksWidget* menu = new BookmarksWidget(m_bookmarksModel->bookmarkId(url), p_QupZilla->locationBar());
        menu->showAt(this);
    } else
        m_bookmarksModel->saveBookmark(p_QupZilla->weView());
}

void BookmarkIcon::checkBookmark(const QUrl &url)
{
    if (m_lastUrl == url)
        return;

    if (m_bookmarksModel->isBookmarked(url))
        setBookmarkSaved();
     else
        setBookmarkDisabled();

    m_lastUrl = url;
}

void BookmarkIcon::bookmarkDeleted(const BookmarksModel::Bookmark &bookmark)
{
    if (bookmark.url == m_lastUrl)
        setBookmarkDisabled();
}

void BookmarkIcon::bookmarkAdded(const BookmarksModel::Bookmark &bookmark)
{
    if (bookmark.url == m_lastUrl)
        setBookmarkSaved();
}

void BookmarkIcon::setBookmarkSaved()
{
    setProperty("bookmarked", true);
    style()->unpolish(this);
    style()->polish(this);
    setToolTip(tr("Edit this bookmark"));
}

void BookmarkIcon::setBookmarkDisabled()
{
    setProperty("bookmarked", false);
    style()->unpolish(this);
    style()->polish(this);
    setToolTip(tr("Bookmark this Page"));
}
