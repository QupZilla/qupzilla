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
#include "locationbar.h"
#include "qupzilla.h"
#include "webview.h"
#include "rssmanager.h"
#include "mainapplication.h"
#include "locationcompleter.h"
#include "clickablelabel.h"
#include "siteinfowidget.h"
#include "rsswidget.h"
#include "webpage.h"
#include "bookmarkicon.h"
#include "progressbar.h"
#include "statusbarmessage.h"
#include "locationbarsettings.h"
#include "toolbutton.h"
#include "searchenginesmanager.h"
#include "siteicon.h"

LocationBar::LocationBar(QupZilla* mainClass)
    : LineEdit()
    , p_QupZilla(mainClass)
    , m_webView(0)
    , m_locationBarSettings(LocationBarSettings::instance())
{
    setObjectName("locationbar");
    m_siteIcon = new SiteIcon(this);

    m_rssIcon = new ClickableLabel(this);
    m_rssIcon->setObjectName("locationbar-rss-icon");
    m_rssIcon->setCursor(Qt::PointingHandCursor);
    m_rssIcon->setToolTip(tr("Add RSS from this page..."));
    m_rssIcon->setFocusPolicy(Qt::ClickFocus);
    m_rssIcon->setVisible(false);

    m_goButton = new ClickableLabel(this);
    m_goButton->setObjectName("locationbar-goicon");
    m_goButton->setCursor(Qt::PointingHandCursor);
    m_goButton->setHidden(true);

    m_bookmarkIcon = new BookmarkIcon(p_QupZilla);

    ClickableLabel* down = new ClickableLabel(this);
    down->setObjectName("locationbar-down-icon");
    down->setCursor(Qt::ArrowCursor);

    addWidget(down, LineEdit::RightSide);
    addWidget(m_bookmarkIcon, LineEdit::RightSide);
    addWidget(m_goButton, LineEdit::RightSide);
    addWidget(m_rssIcon, LineEdit::RightSide);

    setWidgetSpacing(0);

    m_locationCompleter = new LocationCompleter();
    setCompleter(m_locationCompleter);

//    LocationPopup* com = new LocationPopup(this);
//    connect(down, SIGNAL(clicked(QPoint)), com, SLOT(show()));

    connect(this, SIGNAL(textEdited(QString)), this, SLOT(textEdit()));
    connect(this, SIGNAL(textEdited(QString)), m_locationCompleter, SLOT(refreshCompleter(QString)));
//    connect(this, SIGNAL(returnPressed()), this, SLOT(urlEnter()));
    connect(m_locationCompleter->popup(), SIGNAL(clicked(QModelIndex)), p_QupZilla, SLOT(urlEnter()));
    connect(m_siteIcon, SIGNAL(clicked()), this, SLOT(showSiteInfo()));
    connect(m_goButton, SIGNAL(clicked(QPoint)), this, SLOT(urlEnter()));
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

void LocationBar::urlEnter()
{
    QUrl urlToLoad;

    //Check for Search Engine shortcut
    int firstSpacePos = text().indexOf(" ");
    if (firstSpacePos != -1) {
        QString shortcut = text().mid(0, firstSpacePos);
        QString searchedString = text().mid(firstSpacePos).trimmed();

        SearchEngine en = mApp->searchEnginesManager()->engineForShortcut(shortcut);
        if (!en.name.isEmpty()) {
            urlToLoad = en.url.replace("%s", searchedString);
        }
    }

    if (urlToLoad.isEmpty()) {
        QUrl guessedUrl = WebView::guessUrlFromString(text());
        if (!guessedUrl.isEmpty()) {
            urlToLoad = guessedUrl;
        }
        else {
            urlToLoad = text();
        }
    }

    m_webView->setFocus();
    emit loadUrl(urlToLoad);
}

void LocationBar::textEdit()
{
    m_locationCompleter->popup()->setUpdatesEnabled(false);
    showGoButton();
}

void LocationBar::showGoButton()
{
    if (m_goButton->isVisible()) {
        return;
    }

    m_rssIconVisible = m_rssIcon->isVisible();

    m_bookmarkIcon->hide();
    m_rssIcon->hide();
    m_goButton->show();
}

void LocationBar::hideGoButton()
{
    if (!m_goButton->isVisible()) {
        return;
    }

    m_rssIcon->setVisible(m_rssIconVisible);
    m_bookmarkIcon->show();
    m_goButton->hide();
}

void LocationBar::showMostVisited()
{
    if (text().isEmpty()) {
        // Workaround: If we show popup when text in locationbar is empty and then
        // move up and down in completer and then we leave completer -> completer will
        // set text in locationbar back to last "real" completion
        keyPressEvent(new QKeyEvent(QEvent::KeyPress, Qt::Key_unknown, Qt::NoModifier, QString(" ")));
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
}

void LocationBar::showUrl(const QUrl &url, bool empty)
{
    if (hasFocus() || (url.isEmpty() && empty)) {
        return;
    }

    QString encodedUrl = url.toEncoded();

    if (url.toString() == "qupzilla:speeddial") {
        encodedUrl = "";
    }

    if (url.toEncoded() != text()) {
        setText(encodedUrl);
    }
    p_QupZilla->statusBarMessage()->clearMessage();

    hideGoButton();

    m_bookmarkIcon->checkBookmark(url);
}

void LocationBar::siteIconChanged()
{
    QIcon icon_ = m_webView->siteIcon();

    if (icon_.isNull()) {
        clearIcon();
    }
    else {
        m_siteIcon->setIcon(QIcon(icon_.pixmap(16, 16)));
    }
}

void LocationBar::clearIcon()
{
    m_siteIcon->setIcon(QIcon(QWebSettings::webGraphic(QWebSettings::DefaultFrameIconGraphic)));
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

void LocationBar::dropEvent(QDropEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        QUrl dropUrl = event->mimeData()->urls().at(0);
        if (WebView::isUrlValid(dropUrl)) {
            setText(dropUrl.toString());

            m_webView->setFocus();
            emit loadUrl(dropUrl);

            return;
        }
    }
    else if (event->mimeData()->hasText()) {
        QUrl dropUrl = QUrl(event->mimeData()->text().trimmed());
        if (WebView::isUrlValid(dropUrl)) {
            setText(dropUrl.toString());

            m_webView->setFocus();
            emit loadUrl(dropUrl);

            return;
        }

    }
    QLineEdit::dropEvent(event);
}

void LocationBar::focusOutEvent(QFocusEvent* e)
{
    QLineEdit::focusOutEvent(e);
    if (!selectedText().isEmpty() && e->reason() != Qt::TabFocusReason) {
        return;
    }

    setCursorPosition(0);
    hideGoButton();
}

void LocationBar::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_locationBarSettings->selectAllOnDoubleClick) {
        selectAll();
    }
    else {
        QLineEdit::mouseDoubleClickEvent(event);
    }
}

void LocationBar::mousePressEvent(QMouseEvent* event)
{
    if (cursorPosition() == 0 && m_locationBarSettings->selectAllOnClick) {
        selectAll();
        return;
    }

    LineEdit::mousePressEvent(event);
}

void LocationBar::keyPressEvent(QKeyEvent* event)
{
    static QString localDomain = tr(".co.uk", "Append domain name on ALT + Enter = Should be different for every country");

    switch (event->key()) {
    case Qt::Key_Escape:
        setText(m_webView->url().toEncoded());
        event->accept();
        break;

    case Qt::Key_Alt:
        if (event->key() == Qt::Key_Alt && m_locationBarSettings->addCountryWithAlt && !text().endsWith(localDomain) && !text().endsWith("/")) {
            setText(text().append(localDomain));
        }

        LineEdit::keyPressEvent(event);
        break;

    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (event->modifiers() == Qt::ControlModifier) {
            setText(text().append(".com"));
            urlEnter();
        }
        else {
            urlEnter();
        }
        break;

    default:
        LineEdit::keyPressEvent(event);
    }
}

LocationBar::~LocationBar()
{
    delete m_bookmarkIcon;
    delete m_locationCompleter;
}
