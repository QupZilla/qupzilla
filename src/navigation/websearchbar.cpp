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
#include "websearchbar.h"
#include "qupzilla.h"
#include "mainapplication.h"
#include "tabbedwebview.h"
#include "webpage.h"
#include "tabwidget.h"
#include "clickablelabel.h"
#include "buttonwithmenu.h"
#include "searchenginesmanager.h"
#include "searchenginesdialog.h"
#include "networkmanager.h"

WebSearchBar::WebSearchBar(QupZilla* mainClass, QWidget* parent)
    : LineEdit(parent)
    , p_QupZilla(mainClass)
    , m_menu(new QMenu(this))
    , m_pasteAndGoAction(0)
    , m_clearAction(0)
{
    setObjectName("websearchbar");
    m_buttonSearch = new ClickableLabel(this);
    m_buttonSearch->setObjectName("websearchbar-searchbutton");
    m_buttonSearch->setCursor(QCursor(Qt::PointingHandCursor));
    m_buttonSearch->setFocusPolicy(Qt::ClickFocus);

    m_boxSearchType = new ButtonWithMenu(this);
    m_boxSearchType->setObjectName("websearchbar-searchprovider-comobobox");

    addWidget(m_buttonSearch, LineEdit::RightSide);

    connect(this, SIGNAL(returnPressed()), this, SLOT(search()));
    connect(m_buttonSearch, SIGNAL(clicked(QPoint)), this, SLOT(search()));
    connect(m_buttonSearch, SIGNAL(middleClicked(QPoint)), this, SLOT(searchInNewTab()));
    connect(m_boxSearchType, SIGNAL(activeItemChanged(ButtonWithMenu::Item)), this, SLOT(searchChanged(ButtonWithMenu::Item)));

    setWidgetSpacing(0);

    m_searchManager = mApp->searchEnginesManager();
    connect(m_boxSearchType->menu(), SIGNAL(aboutToShow()), this, SLOT(aboutToShowMenu()));

    m_completer = new QCompleter(this);
    m_completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    m_completerModel = new QStringListModel(this);
    m_completer->setModel(m_completerModel);
    m_completer->popup()->setMinimumHeight(90);
    setCompleter(m_completer);

    m_openSearchEngine = new OpenSearchEngine(this);
    m_openSearchEngine->setNetworkAccessManager(mApp->networkManager());
    connect(m_openSearchEngine, SIGNAL(suggestions(const QStringList &)), this, SLOT(addSuggestions(const QStringList &)));
    connect(this, SIGNAL(textEdited(const QString &)), m_openSearchEngine, SLOT(requestSuggestions(QString)));

    QTimer::singleShot(0, this, SLOT(setupEngines()));
}

void WebSearchBar::aboutToShowMenu()
{
    QMenu* menu = m_boxSearchType->menu();

    menu->addSeparator();
    completeMenuWithAvailableEngines(menu);
    menu->addSeparator();
    menu->addAction(QIcon(":icons/menu/gear.png"), tr("Manage Search Engines"), this, SLOT(openSearchEnginesDialog()));
}

void WebSearchBar::addSuggestions(const QStringList &list)
{
    QStringList list_ = list.mid(0, 6);
    m_completerModel->setStringList(list_);
}

void WebSearchBar::openSearchEnginesDialog()
{
    if (m_searchDialog) {
        m_searchDialog.data()->raise();
        m_searchDialog.data()->activateWindow();
        return;
    }

    m_searchDialog = new SearchEnginesDialog(this);
    m_searchDialog.data()->show();
}

void WebSearchBar::setupEngines()
{
    disconnect(m_searchManager, SIGNAL(enginesChanged()), this, SLOT(setupEngines()));

    QString activeEngine = m_searchManager->startingEngineName();

    if (m_boxSearchType->allItems().count() != 0) {
        activeEngine = m_activeEngine.name;
    }

    m_boxSearchType->clearItems();

    foreach(const SearchEngine &en, m_searchManager->allEngines()) {
        ButtonWithMenu::Item item;
        item.icon = en.icon;
        item.text = en.name;
        QVariant v;
        v.setValue<SearchEngine>(en);
        item.userData = v;

        m_boxSearchType->addItem(item);

        if (item.text == activeEngine) {
            m_boxSearchType->setCurrentItem(item);
        }
    }

    connect(m_searchManager, SIGNAL(enginesChanged()), this, SLOT(setupEngines()));
}

void WebSearchBar::searchChanged(const ButtonWithMenu::Item &item)
{
    setPlaceholderText(item.text);
    m_completerModel->setStringList(QStringList());

    m_activeEngine = item.userData.value<SearchEngine>();

    m_openSearchEngine->setSuggestionsUrl(m_activeEngine.suggestionsUrl);
    m_openSearchEngine->setSuggestionsParameters(m_activeEngine.suggestionsParameters);

    m_searchManager->setActiveEngine(m_activeEngine);
}

void WebSearchBar::search()
{
    p_QupZilla->weView()->setFocus();
    p_QupZilla->weView()->load(m_searchManager->searchUrl(m_activeEngine, text()));
}

void WebSearchBar::searchInNewTab()
{
    p_QupZilla->weView()->setFocus();
    p_QupZilla->tabWidget()->addView(m_searchManager->searchUrl(m_activeEngine, text()), Qz::NT_NotSelectedTab);
}

void WebSearchBar::completeMenuWithAvailableEngines(QMenu* menu)
{
    WebView* view = p_QupZilla->weView();
    QWebFrame* frame = view->page()->mainFrame();

    QWebElementCollection elements = frame->documentElement().findAll(QLatin1String("link[rel=search]"));
    foreach(const QWebElement &element, elements) {
        if (element.attribute("type") != "application/opensearchdescription+xml") {
            continue;
        }
        QString url = view->url().resolved(element.attribute("href")).toString();
        QString title = element.attribute("title");

        if (url.isEmpty()) {
            continue;
        }
        if (title.isEmpty()) {
            title = view->title();
        }

        menu->addAction(view->icon(), tr("Add %1 ...").arg(title), this, SLOT(addEngineFromAction()))->setData(url);
    }
}

void WebSearchBar::addEngineFromAction()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        m_searchManager->addEngine(action->data().toUrl());
    }
}

void WebSearchBar::pasteAndGo()
{
    clear();
    paste();
    search();
}

void WebSearchBar::contextMenuEvent(QContextMenuEvent* event)
{
    Q_UNUSED(event)

    if (!m_pasteAndGoAction) {
        m_pasteAndGoAction = new QAction(QIcon::fromTheme("edit-paste"), tr("Paste And &Search"), this);
        m_pasteAndGoAction->setShortcut(QKeySequence("Ctrl+Shift+V"));
        connect(m_pasteAndGoAction, SIGNAL(triggered()), this, SLOT(pasteAndGo()));
    }

    if (!m_clearAction) {
        m_clearAction = new QAction(QIcon::fromTheme("edit-clear"), tr("Clear All"), this);
        connect(m_clearAction, SIGNAL(triggered()), this, SLOT(clear()));
    }

    QMenu* tempMenu = createStandardContextMenu();
    m_menu->clear();

    int i = 0;
    foreach(QAction* act, tempMenu->actions()) {
        act->setParent(m_menu);
        tempMenu->removeAction(act);
        m_menu->addAction(act);

        switch (i) {
        case 0:
            act->setIcon(QIcon::fromTheme("edit-undo"));
            break;
        case 1:
            act->setIcon(QIcon::fromTheme("edit-redo"));
            break;
        case 3:
            act->setIcon(QIcon::fromTheme("edit-cut"));
            break;
        case 4:
            act->setIcon(QIcon::fromTheme("edit-copy"));
            break;
        case 5:
            act->setIcon(QIcon::fromTheme("edit-paste"));
            m_menu->addAction(act);
            m_menu->addAction(m_pasteAndGoAction);
            break;
        case 6:
            act->setIcon(QIcon::fromTheme("edit-delete"));
            m_menu->addAction(act);
            m_menu->addAction(m_clearAction);
            break;
        case 8:
            act->setIcon(QIcon::fromTheme("edit-select-all"));
            break;
        }
        ++i;
    }

    delete tempMenu;

    m_pasteAndGoAction->setEnabled(!QApplication::clipboard()->text().isEmpty());

    //Prevent choosing first option with double rightclick
    QPoint pos = QCursor::pos();
    QPoint p(pos.x(), pos.y() + 1);
    m_menu->popup(p);
}

void WebSearchBar::focusOutEvent(QFocusEvent* e)
{
    if (text().isEmpty()) {
        QString search = m_boxSearchType->currentItem()->text;
        setPlaceholderText(search);
    }
    QLineEdit::focusOutEvent(e);
}

void WebSearchBar::focusInEvent(QFocusEvent* e)
{
    QString search = m_boxSearchType->toolTip();

    if (text() == search) {
        clear();
    }

    QLineEdit::focusInEvent(e);
}

void WebSearchBar::dropEvent(QDropEvent* event)
{
    if (event->mimeData()->hasText()) {
        QString dropText = event->mimeData()->text();
        setText(dropText);
        search();
        QLineEdit::focusOutEvent(new QFocusEvent(QFocusEvent::FocusOut));
        return;
    }
    QLineEdit::dropEvent(event);
}

void WebSearchBar::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (event->modifiers() == Qt::AltModifier) {
            searchInNewTab();
        }
        else {
            search();
        }
        break;

    default:
        LineEdit::keyPressEvent(event);
    }
}
