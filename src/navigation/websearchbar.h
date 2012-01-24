/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
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
#ifndef WEBSEARCHBAR_H
#define WEBSEARCHBAR_H

#include <QWidget>
#include <QPushButton>
#include <QToolButton>
#include <QMenu>
#include <QWeakPointer>
#include <QCompleter>
#include <QStringListModel>

#include "lineedit.h"
#include "buttonwithmenu.h"
#include "searchenginesmanager.h"

class QupZilla;
class LineEdit;
class ClickableLabel;
class SearchEnginesManager;
class SearchEnginesDialog;
class OpenSearchEngine;
class WebSearchBar : public LineEdit
{
    Q_OBJECT
    Q_PROPERTY(QSize fixedsize READ size WRITE setFixedSize)
    Q_PROPERTY(int fixedwidth READ width WRITE setFixedWidth)
    Q_PROPERTY(int fixedheight READ height WRITE setFixedHeight)

public:
    explicit WebSearchBar(QupZilla* mainClass, QWidget* parent = 0);

private slots:
    void searchChanged(const ButtonWithMenu::Item &item);
    void setupEngines();

    void search();
    void searchInNewTab();

    void aboutToShowMenu();
    void openSearchEnginesDialog();

    void addSuggestions(const QStringList &list);

    void addEngineFromAction();
    void pasteAndGo();

private:
    void focusInEvent(QFocusEvent* e);
    void focusOutEvent(QFocusEvent* e);
    void dropEvent(QDropEvent* event);
    void keyPressEvent(QKeyEvent* event);

    void completeMenuWithAvailableEngines(QMenu* menu);
    void contextMenuEvent(QContextMenuEvent* event);

    QCompleter* m_completer;
    QStringListModel* m_completerModel;

    OpenSearchEngine* m_openSearchEngine;
    SearchEngine m_activeEngine;

    QupZilla* p_QupZilla;

    ClickableLabel* m_buttonSearch;
    ButtonWithMenu* m_boxSearchType;
    SearchEnginesManager* m_searchManager;
    QWeakPointer<SearchEnginesDialog> m_searchDialog;

    QAction* m_pasteAndGoAction;
    QMenu* m_menu;

};

#endif // WEBSEARCHBAR_H
