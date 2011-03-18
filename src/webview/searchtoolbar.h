/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  nowrep
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
* ============================================================ */
#ifndef SEARCHTOOLBAR_H
#define SEARCHTOOLBAR_H

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
    explicit SearchToolBar(QupZilla* mainClass, QWidget* parent = 0);
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
