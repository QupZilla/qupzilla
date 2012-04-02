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
#include "locationbar.h"
#include "qupzilla.h"
#include "tabbedwebview.h"
#include "rssmanager.h"
#include "mainapplication.h"
#include "locationcompleter.h"
#include "clickablelabel.h"
#include "siteinfowidget.h"
#include "rsswidget.h"
#include "webpage.h"
#include "tabwidget.h"
#include "bookmarkicon.h"
#include "progressbar.h"
#include "statusbarmessage.h"
#include "locationbarsettings.h"
#include "toolbutton.h"
#include "searchenginesmanager.h"
#include "siteicon.h"
#include "goicon.h"
#include "rssicon.h"
#include "downicon.h"
#include "globalfunctions.h"
#include "iconprovider.h"

#include <QClipboard>

LocationBar::LocationBar(QupZilla* mainClass)
    : LineEdit()
    , p_QupZilla(mainClass)
    , m_webView(0)
    , m_menu(new QMenu(this))
    , m_pasteAndGoAction(0)
    , m_clearAction(0)
    , m_holdingAlt(false)
{
    setObjectName("locationbar");

    m_bookmarkIcon = new BookmarkIcon(p_QupZilla);
    m_goIcon = new GoIcon(this);
    m_rssIcon = new RssIcon(this);
    m_rssIcon->setToolTip(tr("Add RSS from this page..."));
    m_siteIcon = new SiteIcon(this);
    DownIcon* down = new DownIcon(this);

    addWidget(down, LineEdit::RightSide);
    addWidget(m_bookmarkIcon, LineEdit::RightSide);
    addWidget(m_goIcon, LineEdit::RightSide);
    addWidget(m_rssIcon, LineEdit::RightSide);

    setWidgetSpacing(0);

    m_locationCompleter = new LocationCompleter();
    setCompleter(m_locationCompleter);

//    LocationPopup* com = new LocationPopup(this);
//    connect(down, SIGNAL(clicked(QPoint)), com, SLOT(show()));

    connect(this, SIGNAL(textEdited(QString)), this, SLOT(textEdit()));
    connect(this, SIGNAL(textEdited(QString)), m_locationCompleter, SLOT(refreshCompleter(QString)));
    connect(m_locationCompleter->popup(), SIGNAL(clicked(QModelIndex)), this, SLOT(urlEnter()));
    connect(m_siteIcon, SIGNAL(clicked()), this, SLOT(showSiteInfo()));
    connect(m_goIcon, SIGNAL(clicked(QPoint)), this, SLOT(urlEnter()));
    connect(m_rssIcon, SIGNAL(clicked(QPoint)), this, SLOT(rssIconClicked()));
    connect(down, SIGNAL(clicked(QPoint)), this, SLOT(showMostVisited()));
    connect(mApp->searchEnginesManager(), SIGNAL(activeEngineChanged()), this, SLOT(updatePlaceHolderText()));

    clearIcon();
    updatePlaceHolderText();
}

void LocationBar::setText(const QString &text)
{
    LineEdit::setText(text);
    setCursorPosition(0);
}

void LocationBar::updatePlaceHolderText()
{
    setPlaceholderText(tr("Enter URL address or search on %1").arg(mApp->searchEnginesManager()->activeEngine().name));
}

QUrl LocationBar::createUrl()
{
    QUrl urlToLoad;

    //Check for Search Engine shortcut
    int firstSpacePos = text().indexOf(" ");
    if (firstSpacePos != -1) {
        QString shortcut = text().mid(0, firstSpacePos);
        QString searchedString = text().mid(firstSpacePos).trimmed();

        SearchEngine en = mApp->searchEnginesManager()->engineForShortcut(shortcut);
        if (!en.name.isEmpty()) {
            urlToLoad = QUrl::fromEncoded(en.url.replace("%s", searchedString).toUtf8());
        }
    }

    if (urlToLoad.isEmpty()) {
        QUrl guessedUrl = WebView::guessUrlFromString(text());
        if (!guessedUrl.isEmpty()) {
            urlToLoad = guessedUrl;
        }
        else {
            urlToLoad = QUrl::fromEncoded(text().toUtf8());
        }
    }

    return urlToLoad;
}

void LocationBar::urlEnter()
{
    m_locationCompleter->popup()->hide();
    m_webView->setFocus();

    emit loadUrl(createUrl());
}

void LocationBar::textEdit()
{
    m_locationCompleter->popup()->setUpdatesEnabled(false);
    showGoButton();
}

void LocationBar::showGoButton()
{
    if (m_goIcon->isVisible()) {
        return;
    }

    m_rssIconVisible = m_rssIcon->isVisible();

    m_bookmarkIcon->hide();
    m_rssIcon->hide();
    m_goIcon->show();

    updateTextMargins();
}

void LocationBar::hideGoButton()
{
    if (!m_goIcon->isVisible()) {
        return;
    }

    m_rssIcon->setVisible(m_rssIconVisible);
    m_bookmarkIcon->show();
    m_goIcon->hide();

    updateTextMargins();
}

void LocationBar::showMostVisited()
{
    if (text().isEmpty()) {
        // Workaround: If we show popup when text in locationbar is empty and then
        // move up and down in completer and then we leave completer -> completer will
        // set text in locationbar back to last "real" completion
        QKeyEvent event(QEvent::KeyPress, Qt::Key_unknown, Qt::NoModifier, QString(" "));
        keyPressEvent(&event);
    }

    m_locationCompleter->showMostVisited();
}

void LocationBar::showSiteInfo()
{
    QUrl url = p_QupZilla->weView()->url();

    if (url.isEmpty() || url.scheme() == "qupzilla") {
        return;
    }

    SiteInfoWidget* info = new SiteInfoWidget(p_QupZilla);
    info->showAt(this);
}

void LocationBar::rssIconClicked()
{
    RSSWidget* rss = new RSSWidget(m_webView, this);
    rss->showAt(this);
}

void LocationBar::showRSSIcon(bool state)
{
    m_rssIcon->setVisible(state);

    updateTextMargins();
}

void LocationBar::showUrl(const QUrl &url)
{
    if (hasFocus() || url.isEmpty()) {
        return;
    }

    QString stringUrl = qz_urlEncodeQueryString(url);

    if (stringUrl == "qupzilla:speeddial" || stringUrl == "about:blank") {
        stringUrl = "";
    }

    if (stringUrl == text()) {
        return;
    }

    setText(stringUrl);

    hideGoButton();
    m_bookmarkIcon->checkBookmark(url);
}

void LocationBar::siteIconChanged()
{
    QIcon icon_ = m_webView->icon();

    if (icon_.isNull()) {
        clearIcon();
    }
    else {
        m_siteIcon->setIcon(QIcon(icon_.pixmap(16, 16)));
    }
}

void LocationBar::clearIcon()
{
    m_siteIcon->setIcon(IconProvider::emptyWebIcon());
}

void LocationBar::setPrivacy(bool state)
{
    m_siteIcon->setProperty("secured", state);
    m_siteIcon->style()->unpolish(m_siteIcon);
    m_siteIcon->style()->polish(m_siteIcon);

    setProperty("secured", state);
    style()->unpolish(this);
    style()->polish(this);
}

void LocationBar::pasteAndGo()
{
    clear();
    paste();
    urlEnter();
}

void LocationBar::contextMenuEvent(QContextMenuEvent* event)
{
    Q_UNUSED(event)

    if (!m_pasteAndGoAction) {
        m_pasteAndGoAction = new QAction(QIcon::fromTheme("edit-paste"), tr("Paste And &Go"), this);
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
    foreach(QAction * act, tempMenu->actions()) {
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

    tempMenu->deleteLater();

    m_pasteAndGoAction->setEnabled(!QApplication::clipboard()->text().isEmpty());

    //Prevent choosing first option with double rightclick
    QPoint pos = event->globalPos();
    QPoint p(pos.x(), pos.y() + 1);
    m_menu->popup(p);
}

void LocationBar::dropEvent(QDropEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        QUrl dropUrl = event->mimeData()->urls().at(0);
        if (WebView::isUrlValid(dropUrl)) {
            setText(dropUrl.toString());

            m_webView->setFocus();
            emit loadUrl(dropUrl);

            QFocusEvent event(QFocusEvent::FocusOut);
            QLineEdit::focusOutEvent(&event);
            return;
        }
    }
    else if (event->mimeData()->hasText()) {
        QUrl dropUrl = QUrl(event->mimeData()->text().trimmed());
        if (WebView::isUrlValid(dropUrl)) {
            setText(dropUrl.toString());

            m_webView->setFocus();
            emit loadUrl(dropUrl);

            QFocusEvent event(QFocusEvent::FocusOut);
            QLineEdit::focusOutEvent(&event);
            return;
        }

    }
    QLineEdit::dropEvent(event);
}

void LocationBar::focusOutEvent(QFocusEvent* e)
{
    QLineEdit::focusOutEvent(e);
    if (e->reason() == Qt::PopupFocusReason || (!selectedText().isEmpty() && e->reason() != Qt::TabFocusReason)) {
        return;
    }

    setCursorPosition(0);
    hideGoButton();

    if (text().trimmed().isEmpty()) {
        clear();
    }
}

void LocationBar::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && LocationBarSettings::selectAllOnDoubleClick) {
        selectAll();
    }
    else {
        QLineEdit::mouseDoubleClickEvent(event);
    }
}

void LocationBar::mousePressEvent(QMouseEvent* event)
{
    if (cursorPosition() == 0 && LocationBarSettings::selectAllOnClick) {
        selectAll();
        return;
    }

    LineEdit::mousePressEvent(event);
}

void LocationBar::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
    case Qt::Key_V:
        if (event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier)) {
            pasteAndGo();
            event->accept();
            return;
        }
        break;

    case Qt::Key_Escape:
        m_webView->setFocus();
        showUrl(m_webView->url());
        event->accept();
        break;

    case Qt::Key_Alt:
        m_holdingAlt = true;
        break;

    case Qt::Key_Return:
    case Qt::Key_Enter:
        switch (event->modifiers()) {
        case Qt::ControlModifier:
            setText(text().append(".com"));
            urlEnter();
            m_holdingAlt = false;
            break;

        case Qt::AltModifier:
            p_QupZilla->tabWidget()->addView(createUrl(), Qz::NT_NotSelectedTab);
            m_holdingAlt = false;
            break;

        default:
            urlEnter();
            m_holdingAlt = false;
        }

        break;

    case Qt::Key_0:
    case Qt::Key_1:
    case Qt::Key_2:
    case Qt::Key_3:
    case Qt::Key_4:
    case Qt::Key_5:
    case Qt::Key_6:
    case Qt::Key_7:
    case Qt::Key_8:
    case Qt::Key_9:
        if (event->modifiers() & Qt::AltModifier || event->modifiers() & Qt::ControlModifier) {
            event->ignore();
            m_holdingAlt = false;
            return;
        }
        break;

    default:
        m_holdingAlt = false;
    }

    LineEdit::keyPressEvent(event);
}

void LocationBar::keyReleaseEvent(QKeyEvent* event)
{
    QString localDomain = tr(".co.uk", "Append domain name on ALT + Enter = Should be different for every country");

    if (event->key() == Qt::Key_Alt && m_holdingAlt && LocationBarSettings::addCountryWithAlt &&
            !text().endsWith(localDomain) && !text().endsWith("/")) {
        LineEdit::setText(text().append(localDomain));
    }

    LineEdit::keyReleaseEvent(event);
}

LocationBar::~LocationBar()
{
    delete m_bookmarkIcon;
    delete m_locationCompleter;
}
