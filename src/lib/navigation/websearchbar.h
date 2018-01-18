/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2018  David Rosca <nowrep@gmail.com>
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

#include <QPointer>

#include "qzcommon.h"
#include "lineedit.h"
#include "buttonwithmenu.h"
#include "searchenginesmanager.h"
#include "clickablelabel.h"

class QStringListModel;

class BrowserWindow;
class LineEdit;
class ClickableLabel;
class SearchEnginesManager;
class SearchEnginesDialog;
class OpenSearchEngine;

class QUPZILLA_EXPORT WebSearchBar_Button : public ClickableLabel
{
public:
    explicit WebSearchBar_Button(QWidget* parent = 0);

private:
    void contextMenuEvent(QContextMenuEvent* event);
};

class QUPZILLA_EXPORT WebSearchBar : public LineEdit
{
    Q_OBJECT

public:
    explicit WebSearchBar(BrowserWindow* window);

private slots:
    void searchChanged(const ButtonWithMenu::Item &item);
    void setupEngines();

    void search();
    void searchInNewTab();

    void aboutToShowMenu();
    void openSearchEnginesDialog();

    void enableSearchSuggestions(bool enable);
    void addSuggestions(const QStringList &list);

    void addEngineFromAction();
    void pasteAndGo();
    void instantSearchChanged(bool);

private:
    void focusOutEvent(QFocusEvent* e);
    void dropEvent(QDropEvent* event);
    void keyPressEvent(QKeyEvent* event);

    void contextMenuEvent(QContextMenuEvent* event);

    QCompleter* m_completer;
    QStringListModel* m_completerModel;

    OpenSearchEngine* m_openSearchEngine;
    SearchEngine m_activeEngine;

    BrowserWindow* m_window;

    WebSearchBar_Button* m_buttonSearch;
    ButtonWithMenu* m_boxSearchType;
    SearchEnginesManager* m_searchManager;
    QPointer<SearchEnginesDialog> m_searchDialog;

    bool m_reloadingEngines;
};

#endif // WEBSEARCHBAR_H
