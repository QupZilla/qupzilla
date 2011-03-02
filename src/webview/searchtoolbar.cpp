#include "searchtoolbar.h"
#include "qupzilla.h"
#include "webview.h"
#include "lineedit.h"

SearchToolBar::SearchToolBar(QupZilla* mainClass, QWidget *parent) :
    QToolBar(parent)
    ,p_QupZilla(mainClass)
    ,m_findFlags(0)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    setObjectName("webSearchToolbar");
    setWindowTitle(tr("Search"));
    setMovable(false);

    m_searchLine = new LineEdit(this);
    m_searchLine->setInactiveText(tr("Search"));
    m_searchLine->setMaximumWidth(250);
    connect(m_searchLine, SIGNAL(returnPressed()), this, SLOT(findNext()));

    m_closeButton = new QAction(this);
#ifdef Q_WS_X11
    m_closeButton->setIcon(QIcon(style()->standardIcon(QStyle::SP_DialogCloseButton).pixmap(16,16)));
#else
    closeButton->setIcon(QIcon(QIcon(":/icons/faenza/close.png").pixmap(16,16)));
#endif
    connect(m_closeButton, SIGNAL(triggered()), this, SLOT(hideBar()));

    m_highlightButton = new QAction(tr("Highlight occurrences"),this);
    m_highlightButton->setCheckable(true);
    connect(m_highlightButton, SIGNAL(triggered(bool)), this, SLOT(refreshFindFlags(bool)));

    m_nextButton = new QAction(tr("Next"),this);
#ifdef Q_WS_X11
    m_nextButton->setIcon(style()->standardIcon(QStyle::SP_ArrowForward));
#else
    nextButton->setIcon(QIcon(":/icons/faenza/forward.png"));
#endif

    connect(m_nextButton, SIGNAL(triggered()), this, SLOT(findNext()));

    m_previousButton = new QAction(tr("Previous"),this);
#ifdef Q_WS_X11
    m_previousButton->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
#else
    previousButton->setIcon(QIcon(":/icons/faenza/back.png"));
#endif
    connect(m_previousButton, SIGNAL(triggered()), this, SLOT(findPrevious()));

    m_caseSensitiveButton = new QAction(tr("Case sensitive"),this);
    m_caseSensitiveButton->setCheckable(true);
    connect(m_caseSensitiveButton, SIGNAL(triggered(bool)), this, SLOT(refreshFindFlags(bool)));

    m_searchResults = new QLabel(this);

    addAction(m_closeButton);
    addWidget(new QLabel(tr("Find:")));
    addWidget(m_searchLine);
    addSeparator();
    addAction(m_previousButton);
    addAction(m_nextButton);
    addAction(m_highlightButton);
    addAction(m_caseSensitiveButton);
    addWidget(m_searchResults);

    frameChanged(0);
    connect(m_searchLine, SIGNAL(textChanged(QString)), this, SLOT(searchText(QString)));

    m_animation = new QTimeLine(300, this);
    connect(m_animation, SIGNAL(frameChanged(int)),this, SLOT(frameChanged(int)));
}

void SearchToolBar::showBar()
{
    setStyleSheet("QLabel, QToolButton {color: "+p_QupZilla->menuTextColor().name()+";}");
    m_animation->setFrameRange(0, 35);
    m_animation->setDirection(QTimeLine::Forward);
    disconnect(m_animation, SIGNAL(finished()),this, SLOT(hide()));

    m_animation->stop();
    m_animation->start();

    m_searchLine->setFocus();

    QToolBar::show();
}

void SearchToolBar::hideBar()
{
    m_animation->setDirection(QTimeLine::Backward);

    m_animation->stop();
    m_animation->start();
    connect(m_animation, SIGNAL(finished()), this, SLOT(hide()));

    m_searchLine->clear();
    p_QupZilla->weView()->setFocus();
}

void SearchToolBar::frameChanged(int frame)
{
    setMinimumHeight(frame);
    setMaximumHeight(frame);
}

void SearchToolBar::findNext()
{
    refreshFindFlags(true);
    m_findFlags+=4;
    searchText(m_searchLine->text());

}

void SearchToolBar::findPrevious()
{
    refreshFindFlags(true);
    m_findFlags+=5;
    searchText(m_searchLine->text());
}

void SearchToolBar::refreshFindFlags(bool b)
{
    Q_UNUSED(b);
    m_findFlags = 0;
    if (m_highlightButton->isChecked()) {
        m_findFlags+=8;
        searchText(m_searchLine->text());
    }else{
        m_findFlags+=8;
        searchText("");
        m_findFlags-=8;
    }
    if (m_caseSensitiveButton->isChecked()) {
        m_findFlags+=2;
        searchText(m_searchLine->text());
    }
}

void SearchToolBar::searchText(const QString &text)
{
    bool found = p_QupZilla->weView()->findText(text, QFlags<QWebPage::FindFlag>(m_findFlags));
    if (!found && !m_searchLine->text().isEmpty()) {
        m_searchLine->setStyleSheet("background-color: #ff6666;");
        m_searchResults->setText(tr("No results found."));
    }
    else{
        m_searchLine->setStyleSheet("");
        m_searchResults->clear();
    }
}
