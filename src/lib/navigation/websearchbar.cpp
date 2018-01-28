/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018 David Rosca <nowrep@gmail.com>
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
#include "browserwindow.h"
#include "mainapplication.h"
#include "tabbedwebview.h"
#include "webpage.h"
#include "settings.h"
#include "qzsettings.h"
#include "tabwidget.h"
#include "clickablelabel.h"
#include "buttonwithmenu.h"
#include "searchenginesmanager.h"
#include "searchenginesdialog.h"
#include "networkmanager.h"
#include "iconprovider.h"
#include "scripts.h"

#include <QMimeData>
#include <QAbstractItemView>
#include <QCompleter>
#include <QStringListModel>
#include <QMenu>
#include <QTimer>
#include <QClipboard>
#include <QContextMenuEvent>

WebSearchBar_Button::WebSearchBar_Button(QWidget* parent)
    : ClickableLabel(parent)
{
    setObjectName("websearchbar-searchbutton");
    setCursor(QCursor(Qt::PointingHandCursor));
    setFocusPolicy(Qt::ClickFocus);
}

void WebSearchBar_Button::contextMenuEvent(QContextMenuEvent* event)
{
    event->accept();
}

WebSearchBar::WebSearchBar(BrowserWindow* window)
    : LineEdit(window)
    , m_window(window)
    , m_reloadingEngines(false)
{
    setObjectName("websearchbar");
    setDragEnabled(true);

    m_buttonSearch = new WebSearchBar_Button(this);

    m_boxSearchType = new ButtonWithMenu(this);
    m_boxSearchType->setObjectName("websearchbar-searchprovider-combobox");
    // RTL Support
    // If we don't add 'm_boxSearchType' by following code, then we should use suitable padding-left value
    // but then, when typing RTL text the layout dynamically changed and within RTL layout direction
    // padding-left is equivalent to padding-right and vice versa, and because style sheet is
    // not changed dynamically this create padding problems.
    addWidget(m_boxSearchType, LineEdit::LeftSide);

    addWidget(m_buttonSearch, LineEdit::RightSide);

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
    connect(m_completer->popup(), &QAbstractItemView::activated, this, &WebSearchBar::search);

    m_openSearchEngine = new OpenSearchEngine(this);
    m_openSearchEngine->setNetworkAccessManager(mApp->networkManager());
    connect(m_openSearchEngine, SIGNAL(suggestions(QStringList)), this, SLOT(addSuggestions(QStringList)));
    connect(this, SIGNAL(textEdited(QString)), m_openSearchEngine, SLOT(requestSuggestions(QString)));

    editAction(PasteAndGo)->setText(tr("Paste And &Search"));
    editAction(PasteAndGo)->setIcon(QIcon::fromTheme(QSL("edit-paste")));
    connect(editAction(PasteAndGo), SIGNAL(triggered()), this, SLOT(pasteAndGo()));

    QTimer::singleShot(0, this, SLOT(setupEngines()));
}

void WebSearchBar::aboutToShowMenu()
{
    QMenu* menu = m_boxSearchType->menu();

    menu->addSeparator();

    m_window->weView()->page()->runJavaScript(Scripts::getOpenSearchLinks(), WebPage::SafeJsWorld, [this, menu](const QVariant &res) {
        const QVariantList &list = res.toList();
        Q_FOREACH (const QVariant &val, list) {
            const QVariantMap &link = val.toMap();
            QUrl url = m_window->weView()->url().resolved(link.value(QSL("url")).toUrl());
            QString title = link.value(QSL("title")).toString();

            if (url.isEmpty())
                continue;

            if (title.isEmpty())
                title = m_window->weView()->title();

            menu->addAction(m_window->weView()->icon(), tr("Add %1 ...").arg(title), this, SLOT(addEngineFromAction()))->setData(url);
        }

        menu->addSeparator();
        menu->addAction(IconProvider::settingsIcon(), tr("Manage Search Engines"), this, SLOT(openSearchEnginesDialog()));
    });
}

void WebSearchBar::addSuggestions(const QStringList &list)
{
    if (qzSettings->showWSBSearchSuggestions) {
        QStringList list_ = list.mid(0, 6);
        m_completerModel->setStringList(list_);
        m_completer->complete();
    }
}

void WebSearchBar::openSearchEnginesDialog()
{
    if (!m_searchDialog)
        m_searchDialog = new SearchEnginesDialog(this);

    m_searchDialog->open();
    m_searchDialog->raise();
    m_searchDialog->activateWindow();
}

void WebSearchBar::enableSearchSuggestions(bool enable)
{
    Settings settings;
    settings.beginGroup("SearchEngines");
    settings.setValue("showSuggestions", enable);
    settings.endGroup();

    qzSettings->showWSBSearchSuggestions = enable;
    m_completerModel->setStringList(QStringList());
}

void WebSearchBar::setupEngines()
{
    disconnect(m_searchManager, SIGNAL(enginesChanged()), this, SLOT(setupEngines()));
    m_reloadingEngines = true;

    QString activeEngine = m_searchManager->startingEngineName();

    if (m_boxSearchType->allItems().count() != 0) {
        activeEngine = m_activeEngine.name;
    }

    m_boxSearchType->clearItems();

    foreach (const SearchEngine &en, m_searchManager->allEngines()) {
        ButtonWithMenu::Item item;
        item.icon = en.icon;
        item.text = en.name;
        QVariant v;
        v.setValue<SearchEngine>(en);
        item.userData = v;

        m_boxSearchType->addItem(item);

        if (item.text == activeEngine) {
            m_boxSearchType->setCurrentItem(item, false);
        }
    }

    searchChanged(m_boxSearchType->currentItem());

    connect(m_searchManager, SIGNAL(enginesChanged()), this, SLOT(setupEngines()));
    m_reloadingEngines = false;
}

void WebSearchBar::searchChanged(const ButtonWithMenu::Item &item)
{
    setPlaceholderText(item.text);
    m_completerModel->setStringList(QStringList());

    m_activeEngine = item.userData.value<SearchEngine>();

    m_openSearchEngine->setSuggestionsUrl(m_activeEngine.suggestionsUrl);
    m_openSearchEngine->setSuggestionsParameters(m_activeEngine.suggestionsParameters);

    m_searchManager->setActiveEngine(m_activeEngine);

    if (qzSettings->searchOnEngineChange && !m_reloadingEngines && !text().isEmpty()) {
        search();
    }
}

void WebSearchBar::instantSearchChanged(bool enable)
{
    Settings settings;
    settings.beginGroup("SearchEngines");
    settings.setValue("SearchOnEngineChange", enable);
    settings.endGroup();
    qzSettings->searchOnEngineChange = enable;
}

void WebSearchBar::search()
{
    m_window->weView()->setFocus();
    m_window->weView()->load(m_searchManager->searchResult(m_activeEngine, text()));
}

void WebSearchBar::searchInNewTab()
{
    int index = m_window->tabWidget()->addView(QUrl());
    m_window->weView(index)->setFocus();
    m_window->weView(index)->load(m_searchManager->searchResult(m_activeEngine, text()));
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

    QMenu* menu = createContextMenu();
    menu->setAttribute(Qt::WA_DeleteOnClose);

    menu->addSeparator();
    QAction* act = menu->addAction(tr("Show suggestions"));
    act->setCheckable(true);
    act->setChecked(qzSettings->showWSBSearchSuggestions);
    connect(act, SIGNAL(triggered(bool)), this, SLOT(enableSearchSuggestions(bool)));

    QAction* instantSearch = menu->addAction(tr("Search when engine changed"));
    instantSearch->setCheckable(true);
    instantSearch->setChecked(qzSettings->searchOnEngineChange);
    connect(instantSearch, SIGNAL(triggered(bool)), this, SLOT(instantSearchChanged(bool)));

    // Prevent choosing first option with double rightclick
    QPoint pos = event->globalPos();
    pos.setY(pos.y() + 1);
    menu->popup(pos);
}

void WebSearchBar::focusOutEvent(QFocusEvent* e)
{
    if (text().isEmpty()) {
        QString search = m_boxSearchType->currentItem().text;
        setPlaceholderText(search);
    }

    LineEdit::focusOutEvent(e);
}

void WebSearchBar::dropEvent(QDropEvent* event)
{
    if (event->mimeData()->hasText()) {
        QString dropText = event->mimeData()->text();
        setText(dropText);
        search();

        QFocusEvent event(QFocusEvent::FocusOut);
        LineEdit::focusOutEvent(&event);
        return;
    }

    LineEdit::dropEvent(event);
}

void WebSearchBar::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
    case Qt::Key_V:
        if (event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier)) {
            pasteAndGo();
            event->accept();
            return;
        }
        break;

    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (event->modifiers() == Qt::AltModifier) {
            searchInNewTab();
        }
        else {
            search();
        }
        break;

    case Qt::Key_Up:
        if (event->modifiers() == Qt::ControlModifier) {
            m_boxSearchType->selectPreviousItem();
        }
        break;

    case Qt::Key_Down:
        if (event->modifiers() == Qt::ControlModifier) {
            m_boxSearchType->selectNextItem();
        }
        break;

    default:
        break;
    }

    LineEdit::keyPressEvent(event);
}
