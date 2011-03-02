#include "bookmarkswidget.h"
#include "ui_bookmarkswidget.h"
#include "bookmarksmodel.h"
#include "mainapplication.h"

BookmarksWidget::BookmarksWidget(int bookmarkId, QWidget *parent) :
    QMenu(parent)
    ,ui(new Ui::BookmarksWidget)
    ,m_bookmarkId(bookmarkId)
    ,m_bookmarksModel(0)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    connect(ui->close, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->removeBookmark, SIGNAL(clicked()), this, SLOT(removeBookmark()));
    connect(ui->save, SIGNAL(clicked()), this, SLOT(saveBookmark()));

    m_bookmarksModel = MainApplication::getInstance()->bookmarks();
    loadBookmark();
}

void BookmarksWidget::loadBookmark()
{
    QStringList bookmark = m_bookmarksModel->getBookmark(m_bookmarkId);
    ui->name->setText( bookmark.at(1) );

    // Bookmark folders
    ui->folder->addItem(QIcon(":icons/other/unsortedbookmarks.png"), tr("Unsorted Bookmarks"), "unsorted");
    ui->folder->addItem(style()->standardIcon(QStyle::SP_DirOpenIcon), tr("Bookmarks In Menu"), "bookmarksMenu");
    ui->folder->addItem(style()->standardIcon(QStyle::SP_DirOpenIcon), tr("Bookmarks In ToolBar"), "bookmarksToolbar");
    QSqlQuery query;
    query.exec("SELECT name FROM folders");
    while(query.next())
        ui->folder->addItem(style()->standardIcon(QStyle::SP_DirIcon), query.value(0).toString(), query.value(0).toString());

    ui->folder->setCurrentIndex( ui->folder->findData(bookmark.at(2)) );
    ui->name->setCursorPosition(0);
}

void BookmarksWidget::removeBookmark()
{
    m_bookmarksModel->removeBookmark(m_bookmarkId);
    emit bookmarkDeleted();
    close();
}

void BookmarksWidget::saveBookmark()
{
    m_bookmarksModel->editBookmark(m_bookmarkId, ui->name->text(), ui->folder->itemData(ui->folder->currentIndex()).toString() );
    close();
}

void BookmarksWidget::showAt(QWidget* _parent)
{
    QPoint p = _parent->mapToGlobal(QPoint(0, 0));
    move( (p.x()+_parent->width() ) - width(), p.y() + _parent->height());
    show();
}

BookmarksWidget::~BookmarksWidget()
{
    delete ui;
}
