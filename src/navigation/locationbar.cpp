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
#include "bookmarkswidget.h"
#include "bookmarksmodel.h"
#include "siteinfowidget.h"
#include "rsswidget.h"

LocationBar::LocationBar(QupZilla* mainClass, QWidget* parent)
    : LineEdit(parent)
    ,m_selectAllOnDoubleClick(false)
    ,m_addComWithCtrl(false)
    ,m_addCountryWithAlt(false)
    ,p_QupZilla(mainClass)
    ,m_bookmarksModel(0)
{
    m_siteIcon = new QToolButton(this);
    m_siteIcon->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_siteIcon->setCursor(Qt::ArrowCursor);
    m_siteIcon->setMaximumSize(35, 25);
    m_siteIcon->setMinimumSize(35, 25);
    m_siteIcon->setToolTip(tr("Show informations about this page"));
    m_siteIcon->setStyleSheet("QToolButton{border-image: url(:/icons/locationbar/searchchoose.png); margin-left:2px;}");
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

    m_bookmarkButton = new ClickableLabel(this);
    m_bookmarkButton->setPixmap(QPixmap(":/icons/locationbar/starg.png"));
    m_bookmarkButton->setCursor(Qt::PointingHandCursor);
    m_bookmarkButton->setStyleSheet("margin-bottom: 2px;");
    m_bookmarkButton->setToolTip(tr("Bookmark this Page"));
    m_bookmarkButton->setFocusPolicy(Qt::ClickFocus);

    ClickableLabel* down = new ClickableLabel(this);
    down->setPixmap(QPixmap(":icons/locationbar/arrow-down.gif"));
    down->setCursor(Qt::ArrowCursor);

    addWidget(down, LineEdit::RightSide);
    addWidget(m_bookmarkButton, LineEdit::RightSide);
    addWidget(m_goButton, LineEdit::RightSide);
    addWidget(m_rssIcon, LineEdit::RightSide);

    setPlaceholderText(tr("Enter URL address or search on Google.com"));

    setWidgetSpacing(0);
    this->setMinimumHeight(25);
    this->setMaximumHeight(25);
    loadSettings();

    m_locationCompleter = new LocationCompleter();
    setCompleter(m_locationCompleter);

//    LocationPopup* com = new LocationPopup(this);
    connect(this, SIGNAL(textEdited(QString)), this, SLOT(textEdit()));
    connect(this, SIGNAL(textEdited(QString)), m_locationCompleter, SLOT(refreshCompleter(QString)));
    connect(m_locationCompleter->popup(), SIGNAL(clicked(QModelIndex)), p_QupZilla, SLOT(urlEnter()));
    connect(m_siteIcon, SIGNAL(clicked()), this, SLOT(showSiteInfo()));
//    connect(down, SIGNAL(clicked(QPoint)), com, SLOT(show()));
    connect(m_goButton, SIGNAL(clicked(QPoint)), p_QupZilla, SLOT(urlEnter()));
    connect(m_bookmarkButton, SIGNAL(clicked(QPoint)), this, SLOT(bookmarkIconClicked()));
    connect(m_rssIcon, SIGNAL(clicked(QPoint)), this, SLOT(rssIconClicked()));

    setStyleSheet("QLineEdit { background: transparent; border-image: url(:/icons/locationbar/lineedit.png); border-width:4; color:black;}");
    setLeftMargin(33);
//    setLeftMargin(m_siteIcon->sizeHint().width()+1);
}

void LocationBar::loadSettings()
{
    QSettings settings(p_QupZilla->activeProfil()+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("AddressBar");
    m_selectAllOnDoubleClick = settings.value("SelectAllTextOnDoubleClick",true).toBool();
    m_addComWithCtrl = settings.value("AddComDomainWithCtrlKey",false).toBool();
    m_addCountryWithAlt = settings.value("AddCountryDomainWithAltKey",true).toBool();
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

    m_bookmarkButton->hide();
    m_rssIcon->hide();
    m_goButton->show();
}

void LocationBar::hideGoButton()
{
    if (!m_goButton->isVisible())
        return;

    m_rssIcon->setVisible(m_rssIconVisible);
    m_bookmarkButton->show();
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

void LocationBar::bookmarkIconClicked()
{
    QUrl url = p_QupZilla->weView()->url();

    if (m_bookmarksModel->isBookmarked(url)) {
        BookmarksWidget* menu = new BookmarksWidget(m_bookmarksModel->bookmarkId(url), this);
        menu->showAt(this);
        connect(menu, SIGNAL(bookmarkDeleted()), this, SLOT(checkBookmark()));
    } else if (m_bookmarksModel->saveBookmark(p_QupZilla->weView())) {
        m_bookmarkButton->setPixmap(QPixmap(":/icons/locationbar/star.png"));
        m_bookmarkButton->setToolTip(tr("Edit this bookmark"));
    }
}

void LocationBar::rssIconClicked()
{
    QList<QPair<QString,QString> > _rss = p_QupZilla->weView()->getRss();

    RSSWidget* rss = new RSSWidget(p_QupZilla->weView(), _rss, this);
    rss->showAt(this);
}

void LocationBar::checkBookmark()
{
    if (m_bookmarksModel->isBookmarked(QUrl(text()))) {
        m_bookmarkButton->setPixmap(QPixmap(":/icons/locationbar/star.png"));
        m_bookmarkButton->setToolTip(tr("Edit this bookmark"));
    } else {
        m_bookmarkButton->setPixmap(QPixmap(":/icons/locationbar/starg.png"));
        m_bookmarkButton->setToolTip(tr("Bookmark this Page"));
    }
}

QIcon LocationBar::icon(const QUrl &url)
{
    QUrl url2 = url.scheme() + "://" + url.host();
    url2.host().remove("www");

    QIcon icon = QWebSettings::iconForUrl(url);
    if (icon.isNull())
        icon = QWebSettings::iconForUrl(url2);

    if (icon.isNull())
        icon = QWebSettings::iconForUrl(url2.host().prepend("www"));

    if (!icon.isNull())
        return icon.pixmap(16, 16);
    if (icon.isNull()) {
        QPixmap pixmap = QWebSettings::webGraphic(QWebSettings::DefaultFrameIconGraphic);
        if (pixmap.isNull()) {
            pixmap = QPixmap(":icons/locationbar/unknownpage.png");
            QWebSettings::setWebGraphic(QWebSettings::DefaultFrameIconGraphic, pixmap);
        }
        return pixmap;
    }
    return icon;
}

void LocationBar::showUrl(const QUrl &url, bool empty)
{
    if (url.isEmpty() && empty)
        return;

    if (url.toEncoded()!=text()) {
        setText(url.toEncoded());
        setCursorPosition(0);
    }
    if (url.scheme() == "https")
        setPrivacy(true);
    else setPrivacy(false);

    if (p_QupZilla->weView()->isLoading()) {
        p_QupZilla->ipLabel()->hide();
        p_QupZilla->progressBar()->setVisible(true);
        p_QupZilla->progressBar()->setValue(p_QupZilla->weView()->getLoading());
        p_QupZilla->buttonStop()->setVisible(true);
        p_QupZilla->buttonReload()->setVisible(false);
        p_QupZilla->statusBar()->showMessage(tr("Loading..."));
    }else{
        p_QupZilla->progressBar()->setVisible(false);
        p_QupZilla->buttonStop()->setVisible(false);
        p_QupZilla->buttonReload()->setVisible(true);
        p_QupZilla->statusBar()->showMessage(tr("Done"));
        p_QupZilla->ipLabel()->show();
    }
    hideGoButton();

    if (!m_bookmarksModel)
        m_bookmarksModel = mApp->bookmarks();
    checkBookmark();

    m_rssIcon->setVisible(p_QupZilla->weView()->hasRss());

}

void LocationBar::siteIconChanged()
{
    const QPixmap* icon_ = 0;
    if (!p_QupZilla->weView()->isLoading())
        icon_ = p_QupZilla->weView()->animationLoading( p_QupZilla->tabWidget()->currentIndex(), false)->pixmap();

    if (!icon_) {
        m_siteIcon->setIcon(QIcon(":icons/locationbar/unknownpage.png"));
    } else {
        QIcon icon = *icon_;
        m_siteIcon->setIcon(icon);
    }
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
    if (!selectedText().isEmpty()  && e->reason() != Qt::TabFocusReason)
        return;
    setCursorPosition(0);
    hideGoButton();
}

void LocationBar::dropEvent(QDropEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        QUrl dropUrl = event->mimeData()->urls().at(0);
        if (WebView::isUrlValid(dropUrl)) {
            setText(dropUrl.toEncoded());
            p_QupZilla->loadAddress(dropUrl);
            QLineEdit::focusOutEvent(new QFocusEvent(QFocusEvent::FocusOut));
            return;
        }
    }
    if (event->mimeData()->hasText()) {
        QUrl dropUrl = QUrl(event->mimeData()->text());
        if (WebView::isUrlValid(dropUrl)) {
            setText(dropUrl.toEncoded());
            p_QupZilla->loadAddress(dropUrl);
            QLineEdit::focusOutEvent(new QFocusEvent(QFocusEvent::FocusOut));
            return;
        }

    }
    QLineEdit::dropEvent(event);
}

void LocationBar::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_selectAllOnDoubleClick)
        selectAll();
    else
        QLineEdit::mouseDoubleClickEvent(event);
}

void LocationBar::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        showUrl(p_QupZilla->weView()->url());
        event->accept();
        return;
    }

    QString localDomain = tr(".co.uk","Append domain name on ALT key = Should be different for every country");
    if (event->key() == Qt::Key_Control && m_addComWithCtrl && !text().endsWith(".com")) //Disabled for a while
        setText(text().append(".com"));
    if (event->key() == Qt::Key_Alt && m_addCountryWithAlt && !text().endsWith(localDomain) && !text().endsWith("/"))
        setText(text().append(localDomain));

    QLineEdit::keyPressEvent(event);
}

LocationBar::~LocationBar()
{
    delete m_bookmarkButton;
    delete m_goButton;
    delete m_siteIcon;
    delete m_rssIcon;
    delete m_locationCompleter;
}
