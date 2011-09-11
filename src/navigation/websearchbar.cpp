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
#include "websearchbar.h"
#include "qupzilla.h"
#include "webview.h"
#include "clickablelabel.h"
#include "buttonwithmenu.h"

WebSearchBar::WebSearchBar(QupZilla* mainClass, QWidget* parent)
    :LineEdit(parent)
    ,p_QupZilla(mainClass)
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
    setupSearchTypes();
}

void WebSearchBar::setupSearchTypes()
{
    QList<ButtonWithMenu::Item> items;
    items.append(ButtonWithMenu::Item("Google", QIcon(":/icons/menu/google.png")));
    items.append(ButtonWithMenu::Item("Seznam", QIcon(":/icons/menu/cz_seznam.png")));
    items.append(ButtonWithMenu::Item("Wikipedia (en)", QIcon(":/icons/menu/icon-wikipedia.png")));
    items.append(ButtonWithMenu::Item("Wikipedia (cs)", QIcon(":/icons/menu/icon-wikipedia.png")));
    items.append(ButtonWithMenu::Item("CSFD", QIcon(":/icons/menu/csfd.png")));
    items.append(ButtonWithMenu::Item("Youtube", QIcon(":/icons/menu/youtube.png")));
    m_boxSearchType->addItems(items);
}

void WebSearchBar::searchChanged(const ButtonWithMenu::Item &item)
{
    setPlaceholderText(item.text);
}

void WebSearchBar::search()
{
//    if (text().isEmpty())
//        return;
    ButtonWithMenu::Item* item = m_boxSearchType->activeItem();
    QUrl searchUrl;
    if (item->text == "Google")
        searchUrl = QUrl("http://www.google.com/search?client=qupzilla&q="+text());
    if (item->text == "Seznam")
        searchUrl = QUrl("http://search.seznam.cz/?q="+text());
    if (item->text == "Wikipedia (cs)")
        searchUrl = QUrl("http://cs.wikipedia.org/w/index.php?search="+text());
    if (item->text == "Wikipedia (en)")
        searchUrl = QUrl("http://en.wikipedia.org/w/index.php?search="+text());
    if (item->text == "CSFD")
        searchUrl = QUrl("http://www.csfd.cz/hledat/?q="+text());
    if (item->text == "Youtube")
        searchUrl = QUrl("http://www.youtube.com/results?search_query="+text());

    p_QupZilla->weView()->load(searchUrl);
    p_QupZilla->weView()->setFocus();
}

void WebSearchBar::focusOutEvent(QFocusEvent* e)
{
    if (text().isEmpty()) {
        QString search = m_boxSearchType->activeItem()->text;
        //clear();
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
