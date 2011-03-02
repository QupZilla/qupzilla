#include "bookmarkstoolbar.h"
#include "qupzilla.h"
#include "locationbar.h"
#include "bookmarksmodel.h"

BookmarksToolbar::BookmarksToolbar(QupZilla* mainClass, QWidget *parent) :
    QToolBar(parent)
    ,p_QupZilla(mainClass)
    ,m_bookmarksModel(0)
{
    setObjectName("bookmarksToolbar");
    setWindowTitle(tr("Bookmarks"));
    setStyleSheet("QToolBar{background-image:url(:icons/transp.png); border:none;}");
    setMovable(false);
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customContextMenuRequested(QPoint)));
    QTimer::singleShot(0, this, SLOT(refreshBookmarks()));
}

void BookmarksToolbar::setColor(QColor color)
{
    setStyleSheet("QToolButton {color: "+color.name()+";}");
}

void BookmarksToolbar::customContextMenuRequested(const QPoint &pos)
{
    if (actionAt(pos))
        return;

    QMenu menu;
    menu.addAction(tr("Bookmark Current Page"), p_QupZilla, SLOT(bookmarkPage()));
    menu.addAction(tr("Bookmark All Tabs"), p_QupZilla, SLOT(bookmarkAllTabs()));
    menu.addAction(QIcon::fromTheme("user-bookmarks"), tr("Organize Bookmarks"), p_QupZilla, SLOT(showBookmarksManager()));
    menu.addSeparator();
    menu.addAction(
#ifdef Q_WS_X11
            style()->standardIcon(QStyle::SP_BrowserReload)
#else
            QIcon(":/icons/faenza/reload.png")
#endif
            ,tr("Reload Toolbar"), this, SLOT(refreshBookmarks()));
    menu.addSeparator();
    menu.addAction(m_bookmarksModel->isShowingMostVisited() ? tr("Hide Most Visited") : tr("Show Most Visited"), this, SLOT(showMostVisited()));
    menu.addAction(tr("Hide Toolbar"), this, SLOT(hidePanel()));

    //Prevent choosing first option with double rightclick
    QPoint position = QCursor::pos();
    QPoint p(position.x(), position.y()+1);
    menu.exec(p);
}

void BookmarksToolbar::showMostVisited()
{
    m_bookmarksModel->setShowingMostVisited(!m_bookmarksModel->isShowingMostVisited());
    refreshBookmarks();
}

void BookmarksToolbar::hidePanel()
{
    this->hide();
    p_QupZilla->acShowBookmarksToolbar()->setChecked(false);
}

void BookmarksToolbar::refreshBookmarks()
{
    if (!m_bookmarksModel)
        m_bookmarksModel =MainApplication::getInstance()->bookmarks();

    clear();
    QSqlQuery query;
    query.exec("SELECT title, url FROM bookmarks WHERE folder='bookmarksToolbar'");
    while(query.next()) {
        QUrl url = query.value(1).toUrl();
        QString title = query.value(0).toString();
        QAction* action = new QAction(this);
        if (title.length()>15) {
            title.truncate(13);
            title+="..";
        }

        action->setText(title);
        action->setData(url);
        action->setIcon(LocationBar::icon(url));
        QToolButton* button = new QToolButton(this);
        button->setDefaultAction(action);
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setMaximumHeight(25);
        connect(action, SIGNAL(triggered()), p_QupZilla, SLOT(loadActionUrl()));
        addWidget(button);
    }

    if (!m_bookmarksModel->isShowingMostVisited())
        return;

    QToolButton* mostVis = new QToolButton(this);
    mostVis->setPopupMode(QToolButton::InstantPopup);
    mostVis->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    mostVis->setMaximumHeight(25);
    mostVis->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
    mostVis->setText(tr("Most visited"));
    mostVis->setToolTip(tr("Sites You visited the most"));
    m_menuMostVisited = new QMenu();
    mostVis->setMenu(m_menuMostVisited);
    connect(m_menuMostVisited, SIGNAL(aboutToShow()), this, SLOT(refreshMostVisited()));

    addWidget(mostVis);
}

void BookmarksToolbar::refreshMostVisited()
{
    m_menuMostVisited->clear();

    QSqlQuery query;
    query.exec("SELECT title, url FROM history ORDER BY count DESC LIMIT 10");
    while(query.next()) {
        QUrl url = query.value(1).toUrl();
        QString title = query.value(0).toString();
        if (title.length()>40) {
            title.truncate(40);
            title+="..";
        }
        m_menuMostVisited->addAction(LocationBar::icon(url), title, p_QupZilla, SLOT(loadActionUrl()))->setData(url);
    }
}
