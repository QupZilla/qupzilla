#include "browsinglibrary.h"
#include "ui_browsinglibrary.h"
#include "historymanager.h"
#include "bookmarksmanager.h"
#include "rssmanager.h"
#include "mainapplication.h"

BrowsingLibrary::BrowsingLibrary(QupZilla* mainClass, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BrowsingLibrary)
    , m_historyManager(new HistoryManager(mainClass))
    , m_bookmarksManager(new BookmarksManager(mainClass))
    , m_rssManager(mApp->rssManager())
    , m_historyLoaded(false)
    , m_bookmarksLoaded(false)
    , m_rssLoaded(false)
{
    ui->setupUi(this);
    ui->searchLine->hide();

    //CENTER on scren
    const QRect screen = QApplication::desktop()->screenGeometry();
    const QRect &size = QWidget::geometry();
    QWidget::move( (screen.width()-size.width())/2, (screen.height()-size.height())/2 );

    ui->tabs->AddTab(m_historyManager, QIcon(":/icons/other/bighistory.png"), tr("History"));
    ui->tabs->AddTab(m_bookmarksManager, QIcon(":/icons/other/bigstar.png"), tr("Bookmarks"));
    ui->tabs->AddTab(m_rssManager, QIcon(":/icons/other/bigrss.png"), tr("RSS"));

    ui->tabs->SetMode(FancyTabWidget::Mode_LargeSidebar);
    ui->tabs->SetBackgroundPixmap(QPixmap(":icons/other/background.png"));

    connect(ui->tabs, SIGNAL(CurrentChanged(int)), this, SLOT(currentIndexChanged(int)));
    connect(ui->searchLine, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(search()));
}

void BrowsingLibrary::currentIndexChanged(int index)
{
    switch (index) {
    case 0:
        if (!m_historyLoaded) {
            m_historyManager->refreshTable();
            m_historyLoaded = true;
        }
        ui->searchLine->show();
        break;

    case 1:
        if (!m_bookmarksLoaded) {
            m_bookmarksManager->refreshTable();
            m_bookmarksLoaded = true;
        }
        ui->searchLine->hide();
        break;

    case 2:
        if (!m_rssLoaded) {
            m_rssManager->refreshTable();
            m_rssLoaded = true;
        }
        ui->searchLine->hide();
        break;

    default:
        qWarning("BrowsingLibrary::currentIndexChanged() received index out of range!");
    }
}

void BrowsingLibrary::search()
{
    m_historyManager->search(ui->searchLine->text());
}

void BrowsingLibrary::showHistory(QupZilla* mainClass)
{
    ui->tabs->SetCurrentIndex(0);
    show();
    m_historyManager->setMainWindow(mainClass);

    if (!m_historyLoaded) {
        m_historyManager->refreshTable();
        m_historyLoaded = true;
    }
}

void BrowsingLibrary::showBookmarks(QupZilla* mainClass)
{
    ui->tabs->SetCurrentIndex(1);
    show();
    m_bookmarksManager->setMainWindow(mainClass);

    if (!m_bookmarksLoaded) {
        m_bookmarksManager->refreshTable();
        m_bookmarksLoaded = true;
    }
}

void BrowsingLibrary::showRSS(QupZilla* mainClass)
{
    ui->tabs->SetCurrentIndex(2);
    show();
    m_rssManager->setMainWindow(mainClass);
    m_rssManager->refreshTable();
    m_rssLoaded = true;
}

BrowsingLibrary::~BrowsingLibrary()
{
    delete ui;
}
