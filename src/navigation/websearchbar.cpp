/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca <nowrep@gmail.com>
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
#include "webview.h"
#include "webpage.h"
#include "clickablelabel.h"
#include "buttonwithmenu.h"
#include "searchenginesmanager.h"
#include "searchenginesdialog.h"
#include "networkmanager.h"

WebSearchBar::WebSearchBar(QupZilla* mainClass, QWidget* parent)
    : LineEdit(parent)
    , p_QupZilla(mainClass)
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
    if (m_searchDialog.data()) {
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

    foreach(SearchEngine en, m_searchManager->allEngines()) {
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
    p_QupZilla->weView()->load(m_searchManager->searchUrl(m_activeEngine, text()));
    p_QupZilla->weView()->setFocus();
}

void WebSearchBar::completeMenuWithAvailableEngines(QMenu* menu)
{
    WebView* view = p_QupZilla->weView();
    QWebFrame* frame = view->webPage()->mainFrame();

    QWebElementCollection elements = frame->documentElement().findAll(QLatin1String("link[rel=search]"));
    foreach(QWebElement element, elements) {
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
