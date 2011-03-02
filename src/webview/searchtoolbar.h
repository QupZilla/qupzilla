#ifndef SEARCHTOOLBAR_H
#define SEARCHTOOLBAR_H

#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include <QToolBar>
#include <QLineEdit>
#include <QAction>
#include <QWebPage>
#include <QLabel>
#include <QFlags>
#include <QTimeLine>

class QupZilla;
class LineEdit;
class SearchToolBar : public QToolBar
{
    Q_OBJECT
public:
    explicit SearchToolBar(QupZilla* mainClass, QWidget *parent = 0);
    LineEdit* searchLine(){ return m_searchLine; }

signals:

public slots:
    void showBar();
    void hideBar();
    void searchText(const QString &text);
    void refreshFindFlags(bool b);
    void findNext();
    void findPrevious();
    void frameChanged(int frame);

private:
    QupZilla* p_QupZilla;

    LineEdit* m_searchLine;
    QAction* m_closeButton;
    QAction* m_highlightButton;
    QAction* m_caseSensitiveButton;
    QAction* m_nextButton;
    QAction* m_previousButton;
    QLabel* m_searchResults;
    QTimeLine* m_animation;
    int m_findFlags;
};

#endif // SEARCHTOOLBAR_H
