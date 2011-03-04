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

WebSearchBar::WebSearchBar(QupZilla* mainClass, QWidget *parent)
    :LineEdit(parent)
    ,p_QupZilla(mainClass)
{
    m_buttonSearch = new ClickableLabel(this);
    m_buttonSearch->setPixmap(QPixmap(":/icons/locationbar/search.png"));
    m_buttonSearch->setCursor(QCursor(Qt::PointingHandCursor));
    m_buttonSearch->setStyleSheet("QLabel{margin-bottom:2px;}");
    m_buttonSearch->setFocusPolicy(Qt::ClickFocus);

    m_boxSearchType = new QToolButton(this);
    m_boxSearchType->setPopupMode(QToolButton::InstantPopup);
    m_boxSearchType->setCursor(Qt::ArrowCursor);
    m_boxSearchType->setMaximumSize(35, 25);
    m_boxSearchType->setMinimumSize(35, 25);
    m_boxSearchType->setFocusPolicy(Qt::ClickFocus);

    this->setMinimumHeight(25);
    this->setMaximumHeight(25);
    m_boxSearchType->setStyleSheet("QToolButton{border-image: url(:/icons/locationbar/searchchoose.png); padding-left:-6px; margin-left:2px;}"
                                 "QToolButton::menu-indicator {background-image: url(:icons/locationbar/arrow-down.gif); background-repeat: no-repeat;}");

    addWidget(m_buttonSearch, LineEdit::RightSide);

    setupSearchTypes();
    connect(this, SIGNAL(returnPressed()), this, SLOT(search()));
    connect(m_buttonSearch, SIGNAL(clicked(QPoint)), this, SLOT(search()));

    setStyleSheet("QLineEdit { background: transparent; border-image: url(:/icons/locationbar/lineedit.png) ;border-width:4;color:black;}");

    setLeftMargin(30);
    setWidgetSpacing(0);
}

void WebSearchBar::setupSearchTypes()
{
    QMenu* menu = new QMenu(this);
    menu->addAction(QIcon(":/icons/menu/google.png"),"Google", this, SLOT(searchChanged()))->setData("Google");
    menu->addAction(QIcon(":/icons/menu/cz_seznam.png"),"Seznam", this, SLOT(searchChanged()))->setData("Seznam");
    menu->addAction(QIcon(":/icons/menu/icon-wikipedia.png"),"Wikipedia (en)", this, SLOT(searchChanged()))->setData("Wikipedia (en)");
    menu->addAction(QIcon(":/icons/menu/icon-wikipedia.png"),"Wikipedia (cs)", this, SLOT(searchChanged()))->setData("Wikipedia (cs)");
    menu->addAction(QIcon(":/icons/menu/csfd.png"),"CSFD", this, SLOT(searchChanged()))->setData("CSFD");
    menu->addAction(QIcon(":/icons/menu/youtube.png"),"Youtube", this, SLOT(searchChanged()))->setData("Youtube");

    m_boxSearchType->setMenu(menu);
    m_boxSearchType->setIcon(QIcon(":/icons/menu/google.png"));
    m_boxSearchType->setToolTip("Google");

    setPlaceholderText("Google");

}

void WebSearchBar::searchChanged()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        if (action->data().toString() == "Google")
            m_boxSearchType->setIcon(QIcon(":/icons/menu/google.png"));
        else if (action->data().toString() == "Seznam")
            m_boxSearchType->setIcon(QIcon(":/icons/menu/cz_seznam.png"));
        else if (action->data().toString().contains("Wikipedia"))
            m_boxSearchType->setIcon(QIcon(":/icons/menu/icon-wikipedia.png"));
        else if (action->data().toString() == "CSFD")
            m_boxSearchType->setIcon(QIcon(":/icons/menu/csfd.png"));
        else if (action->data().toString() == "Youtube")
            m_boxSearchType->setIcon(QIcon(":/icons/menu/youtube.png"));

        m_boxSearchType->setToolTip(action->data().toString());
        setPlaceholderText(action->data().toString());
    }
}

void WebSearchBar::search()
{
    if (text().isEmpty())
        return;

    QUrl searchUrl;
    if (m_boxSearchType->toolTip() == "Google")
        searchUrl = QUrl("http://www.google.com/search?client=qupzilla&q="+text());
    if (m_boxSearchType->toolTip() == "Seznam")
        searchUrl = QUrl("http://search.seznam.cz/?q="+text());
    if (m_boxSearchType->toolTip() == "Wikipedia (cs)")
        searchUrl = QUrl("http://cs.wikipedia.org/w/index.php?search="+text());
    if (m_boxSearchType->toolTip() == "Wikipedia (en)")
        searchUrl = QUrl("http://en.wikipedia.org/w/index.php?search="+text());
    if (m_boxSearchType->toolTip() == "CSFD")
        searchUrl = QUrl("http://www.csfd.cz/hledani-filmu-hercu-reziseru-ve-filmove-databazi/?search="+text());
    if (m_boxSearchType->toolTip() == "Youtube")
        searchUrl = QUrl("http://www.youtube.com/results?search_query="+text());

    p_QupZilla->weView()->load(searchUrl);
    p_QupZilla->weView()->setFocus();
}

void WebSearchBar::focusOutEvent(QFocusEvent *e)
{
    if (text().isEmpty()) {
        QString search = m_boxSearchType->toolTip();
        //clear();
        setPlaceholderText(search);
    }
    QLineEdit::focusOutEvent(e);
}

void WebSearchBar::focusInEvent(QFocusEvent *e)
{
    QString search = m_boxSearchType->toolTip();

    if (text() == search) {
        clear();
    }
    QLineEdit::focusInEvent(e);
}
