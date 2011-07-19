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

LocationBar::LocationBar(QupZilla* mainClass)
    : LineEdit()
    ,p_QupZilla(mainClass)
    ,m_webView(0)
    ,m_locationBarSettings(LocationBarSettings::instance())
{
    m_siteIcon = new QToolButton(this);
    m_siteIcon->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_siteIcon->setCursor(Qt::ArrowCursor);
    m_siteIcon->setMaximumSize(35, 25);
    m_siteIcon->setMinimumSize(35, 25);
    m_siteIcon->setToolTip(tr("Show informations about this page"));
#if QT_VERSION == 0x040800
    m_siteIcon->setStyleSheet("QToolButton{border-image: url(:/icons/locationbar/searchchoose.png); margin-left:2px; padding-left: 4px; }");
#else
    m_siteIcon->setStyleSheet("QToolButton{border-image: url(:/icons/locationbar/searchchoose.png); margin-left:2px;}");
#endif
    m_siteIcon->setFocusPolicy(Qt::ClickFocus);

    m_rssIcon = new ClickableLabel(this);
    m_rssIcon->setPixmap(QPixmap(":/icons/menu/rss.png"));
    m_rssIcon->setCursor(Qt::PointingHandCursor);
    m_rssIcon->setToolTip(tr("Add RSS from this page..."));
    m_rssIcon->setStyleSheet("margin-bottom:2px");
    m_rssIcon->setFocusPolicy(Qt::ClickFocus);
    m_rssIcon->setVisible(false);

    m_goButton = new ClickableLabel(this);
    m_goButton->setPixmap(QPixmap(":/icons/locationbar/gotoaddress.png"));
    m_goButton->setCursor(Qt::PointingHandCursor);
    m_goButton->setHidden(true);
    m_goButton->setStyleSheet("margin-bottom:2px;");

    m_bookmarkIcon = new BookmarkIcon(p_QupZilla);

    ClickableLabel* down = new ClickableLabel(this);
    down->setPixmap(QPixmap(":icons/locationbar/arrow-down.gif"));
    down->setCursor(Qt::ArrowCursor);

    addWidget(down, LineEdit::RightSide);
    addWidget(m_bookmarkIcon, LineEdit::RightSide);
    addWidget(m_goButton, LineEdit::RightSide);
    addWidget(m_rssIcon, LineEdit::RightSide);

    setPlaceholderText(tr("Enter URL address or search on Google.com"));

    setWidgetSpacing(0);
    this->setMinimumHeight(25);
    this->setMaximumHeight(25);

    m_locationCompleter = new LocationCompleter();
    setCompleter(m_locationCompleter);

//    LocationPopup* com = new LocationPopup(this);
    connect(this, SIGNAL(textEdited(QString)), this, SLOT(textEdit()));
    connect(this, SIGNAL(textEdited(QString)), m_locationCompleter, SLOT(refreshCompleter(QString)));
    connect(this, SIGNAL(returnPressed()), this, SLOT(urlEnter()));
    connect(m_locationCompleter->popup(), SIGNAL(clicked(QModelIndex)), p_QupZilla, SLOT(urlEnter()));
    connect(m_siteIcon, SIGNAL(clicked()), this, SLOT(showSiteInfo()));
//    connect(down, SIGNAL(clicked(QPoint)), com, SLOT(show()));
    connect(m_goButton, SIGNAL(clicked(QPoint)), this, SLOT(urlEnter()));
    connect(m_rssIcon, SIGNAL(clicked(QPoint)), this, SLOT(rssIconClicked()));

    setStyleSheet("QLineEdit { background: transparent; border-image: url(:/icons/locationbar/lineedit.png); border-width:4; color:black;}");
    setLeftMargin(33);
    clearIcon();
//    setLeftMargin(m_siteIcon->sizeHint().width()+1);
}

void LocationBar::urlEnter()
{
    m_webView->setFocus();
    QUrl guessedUrl = WebView::guessUrlFromString(text());
    m_webView->load(guessedUrl);
    setText(guessedUrl.toString());
}

void LocationBar::textEdit()
{
    m_locationCompleter->popup()->setUpdatesEnabled(false);
    showGoButton();
}

void LocationBar::showGoButton()
{
    if (m_goButton->isVisible())
        return;

    m_rssIconVisible = m_rssIcon->isVisible();

    m_bookmarkIcon->hide();
    m_rssIcon->hide();
    m_goButton->show();
}

void LocationBar::hideGoButton()
{
    if (!m_goButton->isVisible())
        return;

    m_rssIcon->setVisible(m_rssIconVisible);
    m_bookmarkIcon->show();
    m_goButton->hide();
}

void LocationBar::showPopup()
{
    //TODO: Fix to next version
    return;
    emit textEdited("");
    m_locationCompleter->popup()->showNormal();
}

void LocationBar::showSiteInfo()
{
    SiteInfoWidget* info = new SiteInfoWidget(p_QupZilla);
    info->showAt(this);
}

void LocationBar::rssIconClicked()
{
    QList<QPair<QString,QString> > _rss = m_webView->getRss();

    RSSWidget* rss = new RSSWidget(m_webView, _rss, this);
    rss->showAt(this);
}

void LocationBar::showRSSIcon(bool state)
{
    m_rssIcon->setVisible(state);
}

void LocationBar::showUrl(const QUrl &url, bool empty)
{
    if (hasFocus() || (url.isEmpty() && empty))
        return;

    if (url.toEncoded()!=text()) {
        setText(url.toEncoded());
        setCursorPosition(0);
    }

//    if (m_webView->isLoading()) {
//        p_QupZilla->ipLabel()->hide();
//        p_QupZilla->progressBar()->setVisible(true);
//        p_QupZilla->progressBar()->setValue(m_webView->getLoading());
//        p_QupZilla->buttonStop()->setVisible(true);
//        p_QupZilla->buttonReload()->setVisible(false);
//    } else {
//        p_QupZilla->progressBar()->setVisible(false);
//        p_QupZilla->buttonStop()->setVisible(false);
//        p_QupZilla->buttonReload()->setVisible(true);
//        p_QupZilla->ipLabel()->show();
//    }

    p_QupZilla->statusBarMessage()->clearMessage();

    hideGoButton();

    m_bookmarkIcon->checkBookmark(url);
}

void LocationBar::siteIconChanged()
{
//    const QPixmap* icon_ = 0;
    QIcon icon_;
//    if (!p_QupZilla->weView()->isLoading())
//        icon_ = p_QupZilla->weView()->animationLoading( p_QupZilla->tabWidget()->currentIndex(), false)->pixmap();
        icon_ = m_webView->siteIcon();

    if (icon_.isNull()) {
        clearIcon();
    } else {
//        QIcon icon(*icon_);
        m_siteIcon->setIcon(QIcon(icon_.pixmap(16,16)));
    }
}

void LocationBar::clearIcon()
{
    m_siteIcon->setIcon(QIcon(QWebSettings::webGraphic(QWebSettings::DefaultFrameIconGraphic)));
}

void LocationBar::setPrivacy(bool state)
{
    if (state)
        m_siteIcon->setStyleSheet("QToolButton{border-image: url(:/icons/locationbar/safeline.png); margin-left:2px;}");
    else
        m_siteIcon->setStyleSheet("QToolButton{border-image: url(:/icons/locationbar/searchchoose.png); margin-left:2px;}");
}

void LocationBar::focusOutEvent(QFocusEvent* e)
{
    QLineEdit::focusOutEvent(e);
    if (!selectedText().isEmpty() && e->reason() != Qt::TabFocusReason)
        return;
    setCursorPosition(0);
    hideGoButton();
}

void LocationBar::dropEvent(QDropEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        QUrl dropUrl = event->mimeData()->urls().at(0);
        if (WebView::isUrlValid(dropUrl)) {
            setText(dropUrl.toString());
            p_QupZilla->loadAddress(dropUrl);
            QLineEdit::focusOutEvent(new QFocusEvent(QFocusEvent::FocusOut));
            return;
        }
    }
    else if (event->mimeData()->hasText()) {
        QUrl dropUrl = QUrl(event->mimeData()->text());
        if (WebView::isUrlValid(dropUrl)) {
            setText(dropUrl.toString());
            p_QupZilla->loadAddress(dropUrl);
            QLineEdit::focusOutEvent(new QFocusEvent(QFocusEvent::FocusOut));
            return;
        }

    }
    QLineEdit::dropEvent(event);
}

void LocationBar::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_locationBarSettings->selectAllOnDoubleClick)
        selectAll();
    else
        QLineEdit::mouseDoubleClickEvent(event);
}

void LocationBar::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        setText(m_webView->url().toEncoded());
        event->accept();
        return;
    }

    QString localDomain = tr(".co.uk","Append domain name on ALT key = Should be different for every country");
    if (event->key() == Qt::Key_Control && m_locationBarSettings->addComWithCtrl && !text().endsWith(".com")) //Disabled for a while
        setText(text().append(".com"));
    if (event->key() == Qt::Key_Alt && m_locationBarSettings->addCountryWithAlt && !text().endsWith(localDomain) && !text().endsWith("/"))
        setText(text().append(localDomain));

    QLineEdit::keyPressEvent(event);
}

LocationBar::~LocationBar()
{
    delete m_bookmarkIcon;
    delete m_goButton;
    delete m_siteIcon;
    delete m_rssIcon;
    delete m_locationCompleter;
}
