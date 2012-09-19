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
#include "clickablelabel.h"
#include "siteinfowidget.h"
#include "rsswidget.h"
#include "webpage.h"
#include "tabwidget.h"
#include "bookmarkicon.h"
#include "bookmarkswidget.h"
#include "progressbar.h"
#include "statusbarmessage.h"
#include "toolbutton.h"
#include "searchenginesmanager.h"
#include "siteicon.h"
#include "goicon.h"
#include "rssicon.h"
#include "downicon.h"
#include "globalfunctions.h"
#include "iconprovider.h"
#include "qzsettings.h"

#include <QClipboard>
#include <QTimer>
#include <QContextMenuEvent>

LocationBar::LocationBar(QupZilla* mainClass)
    : LineEdit(mainClass)
    , p_QupZilla(mainClass)
    , m_webView(0)
    , m_menu(new QMenu(this))
    , m_pasteAndGoAction(0)
    , m_clearAction(0)
    , m_holdingAlt(false)
    , m_loadProgress(0)
    , m_progressVisible(false)
{
    setObjectName("locationbar");
    setDragEnabled(true);

    m_bookmarkIcon = new BookmarkIcon(p_QupZilla);
    m_goIcon = new GoIcon(this);
    m_rssIcon = new RssIcon(this);
    m_rssIcon->setToolTip(tr("Add RSS from this page..."));
    m_siteIcon = new SiteIcon(this);
    DownIcon* down = new DownIcon(this);

    ////RTL Support
    ////if we don't add 'm_siteIcon' by following code, then we should use suitable padding-left value
    //// but then, when typing RTL text the layout dynamically changed and within RTL layout direction
    //// padding-left is equivalent to padding-right and vice versa, and because style sheet is
    //// not changed dynamically this create padding problems.
    addWidget(m_siteIcon, LineEdit::LeftSide);

    addWidget(m_goIcon, LineEdit::RightSide);
    addWidget(m_bookmarkIcon, LineEdit::RightSide);
    addWidget(m_rssIcon, LineEdit::RightSide);
    addWidget(down, LineEdit::RightSide);

    m_completer.setLocationBar(this);
    connect(&m_completer, SIGNAL(showCompletion(QString)), this, SLOT(showCompletion(QString)));
    connect(&m_completer, SIGNAL(completionActivated()), this, SLOT(urlEnter()));

    connect(this, SIGNAL(textEdited(QString)), this, SLOT(textEdit()));
    connect(m_siteIcon, SIGNAL(clicked()), this, SLOT(showSiteInfo()));
    connect(m_goIcon, SIGNAL(clicked(QPoint)), this, SLOT(urlEnter()));
    connect(m_rssIcon, SIGNAL(clicked(QPoint)), this, SLOT(rssIconClicked()));
    connect(m_bookmarkIcon, SIGNAL(clicked(QPoint)), this, SLOT(bookmarkIconClicked()));
    connect(down, SIGNAL(clicked(QPoint)), this, SLOT(showMostVisited()));
    connect(mApp->searchEnginesManager(), SIGNAL(activeEngineChanged()), this, SLOT(updatePlaceHolderText()));
    connect(mApp, SIGNAL(message(Qz::AppMessageType, bool)), SLOT(onMessage(Qz::AppMessageType, bool)));

    loadSettings();

    clearIcon();
    updatePlaceHolderText();
}

void LocationBar::setWebView(TabbedWebView* view)
{
    m_webView = view;

    connect(m_webView, SIGNAL(loadStarted()), SLOT(onLoadStarted()));
    connect(m_webView, SIGNAL(loadProgress(int)), SLOT(onLoadProgress(int)));
    connect(m_webView, SIGNAL(loadFinished(bool)), SLOT(onLoadFinished()));
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

void LocationBar::showCompletion(const QString &newText)
{
    LineEdit::setText(newText);

    // Move cursor to the end
    end(false);
}

QUrl LocationBar::createUrl()
{
    QUrl urlToLoad;

    //Check for Search Engine shortcut
    int firstSpacePos = text().indexOf(QLatin1Char(' '));
    if (firstSpacePos != -1) {
        QString shortcut = text().left(firstSpacePos);
        QString searchedString = QUrl::toPercentEncoding(text().mid(firstSpacePos).trimmed());

        SearchEngine en = mApp->searchEnginesManager()->engineForShortcut(shortcut);
        if (!en.name.isEmpty()) {
            urlToLoad = QUrl::fromEncoded(en.url.replace(QLatin1String("%s"), searchedString).toUtf8());
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

QString LocationBar::convertUrlToText(const QUrl &url) const
{
    QString stringUrl = qz_urlEncodeQueryString(url);

    if (stringUrl == QLatin1String("qupzilla:speeddial") || stringUrl == QLatin1String("about:blank")) {
        stringUrl = "";
    }

    return stringUrl;
}

void LocationBar::urlEnter()
{
    m_completer.closePopup();
    m_webView->setFocus();

    emit loadUrl(createUrl());
}

void LocationBar::textEdit()
{
    if (!text().isEmpty()) {
        m_completer.complete(text());
    }
    else {
        m_completer.closePopup();
    }

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
    m_completer.complete(QString());
}

void LocationBar::showSiteInfo()
{
    QUrl url = p_QupZilla->weView()->url();

    if (url.isEmpty() || url.scheme() == QLatin1String("qupzilla")) {
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

void LocationBar::bookmarkIconClicked()
{
    BookmarksWidget* bWidget = new BookmarksWidget(p_QupZilla, m_webView, this);
    bWidget->showAt(this);
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

    const QString &stringUrl = convertUrlToText(url);

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
    m_siteIcon->setIcon(qIconProvider->emptyWebIcon());
}

void LocationBar::setPrivacy(bool state)
{
    m_siteIcon->setProperty("secured", QVariant(state));
    m_siteIcon->style()->unpolish(m_siteIcon);
    m_siteIcon->style()->polish(m_siteIcon);

    setProperty("secured", QVariant(state));
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

void LocationBar::focusInEvent(QFocusEvent* event)
{
    const QString &stringUrl = convertUrlToText(m_webView->url());

    // Text has been edited, let's show go button
    if (stringUrl != text()) {
        showGoButton();
    }

    LineEdit::focusInEvent(event);
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

void LocationBar::focusOutEvent(QFocusEvent* event)
{
    QLineEdit::focusOutEvent(event);

    if (event->reason() == Qt::PopupFocusReason
            || (!selectedText().isEmpty() && event->reason() != Qt::TabFocusReason)) {
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
    if (event->button() == Qt::LeftButton && qzSettings->selectAllOnDoubleClick) {
        selectAll();
    }
    else {
        QLineEdit::mouseDoubleClickEvent(event);
    }
}

void LocationBar::mousePressEvent(QMouseEvent* event)
{
    if (cursorPosition() == 0 && qzSettings->selectAllOnClick) {
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

    case Qt::Key_Down:
        m_completer.complete(text());
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
            setText(text().append(QLatin1String(".com")));
            urlEnter();
            m_holdingAlt = false;
            break;

        case Qt::AltModifier:
            p_QupZilla->tabWidget()->addView(createUrl());
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

    if (event->key() == Qt::Key_Alt && m_holdingAlt && qzSettings->addCountryWithAlt &&
            !text().endsWith(localDomain) && !text().endsWith(QLatin1Char('/'))) {
        LineEdit::setText(text().append(localDomain));
    }

    LineEdit::keyReleaseEvent(event);
}

LocationBar::~LocationBar()
{
    delete m_bookmarkIcon;
}

void LocationBar::onLoadStarted()
{
    m_progressVisible = true;
}

void LocationBar::onLoadProgress(int progress)
{
    if (qzSettings->showLoadingProgress) {
        m_loadProgress = progress;
        update();
    }
}

void LocationBar::onLoadFinished()
{
    if (qzSettings->showLoadingProgress) {
        QTimer::singleShot(700, this, SLOT(hideProgress()));
    }
}

void LocationBar::loadSettings()
{
    Settings settings;
    settings.beginGroup("AddressBar");
    m_progressStyle = static_cast<ProgressStyle>(settings.value("ProgressStyle", 0).toInt());
    bool customColor = settings.value("UseCustomProgressColor", false).toBool();
    m_progressColor = customColor ? settings.value("CustomProgressColor", palette().color(QPalette::Highlight)).value<QColor>() : QColor();
    settings.endGroup();
}

void LocationBar::onMessage(Qz::AppMessageType msg, bool state)
{
    Q_UNUSED(state)
    if (!qzSettings->showLoadingProgress) {
        return;
    }

    if (msg == Qz::AM_ReloadSettings) {
        loadSettings();
    }
}

void LocationBar::hideProgress()
{
    if (qzSettings->showLoadingProgress) {
        m_progressVisible = false;
        update();
    }
}

void LocationBar::paintEvent(QPaintEvent* event)
{
    if (hasFocus()) {
        LineEdit::paintEvent(event);
        return;
    }

    QStyleOptionFrameV3 option;
    initStyleOption(&option);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    style()->drawPrimitive(QStyle::PE_PanelLineEdit, &option, &p, this);

    QRect contentsRect = style()->subElementRect(QStyle::SE_LineEditContents, &option, this);
    int lm, tm, rm, bm;
    getTextMargins(&lm, &tm, &rm, &bm);
    contentsRect.adjust(lm, tm, -rm, -bm);
    QFontMetrics fm = fontMetrics();
    const int x = contentsRect.x() + 3;
    const int y = contentsRect.y() + (contentsRect.height() - fm.height() + 1) / 2;
    const int width = contentsRect.width() - 6;
    const int height = fm.height();
    QRect textRect(x, y, width, height);

    QPen oldPen = p.pen();

    if (qzSettings->showLoadingProgress && m_progressVisible) {
        QColor bg = m_progressColor;
        if (!bg.isValid() || bg.alpha() == 0) {

            bg = palette().color(QPalette::Base).darker(110);
            if (!bg.isValid() || bg.alpha() == 0) {
                bg = p_QupZilla->palette().color(QPalette::Base).darker(110);
            }
        }
        p.setBrush(QBrush(bg));

        QPen outlinePen(bg.darker(110), 0.8);
        p.setPen(outlinePen);

        switch (m_progressStyle) {
        case ProgressFilled: {
            QRect bar = textRect.adjusted(-3, 0, 6 - (textRect.width() * (100.0 - m_loadProgress) / 100), 0);
            const int roundness = bar.height() / 4.0;
            p.drawRoundedRect(bar, roundness, roundness);
            break;
        }
        case ProgressBottom: {
            outlinePen.setWidthF(0.3);
            outlinePen.setColor(outlinePen.color().darker(130));
            p.setPen(outlinePen);
            QRect bar(contentsRect.x(), contentsRect.bottom() - 2,
                      contentsRect.width()*m_loadProgress / 100.0, 3);
            p.drawRoundedRect(bar, 1, 1);
            break;
        }
        case ProgressTop: {
            outlinePen.setWidthF(0.3);
            outlinePen.setColor(outlinePen.color().darker(130));
            p.setPen(outlinePen);
            QRect bar(contentsRect.x(), contentsRect.top() + 1,
                      contentsRect.width()*m_loadProgress / 100.0, 3);
            p.drawRoundedRect(bar, 1, 1);
            break;
        }
        default:
            break;
        }
    }

    p.setPen(oldPen);
    QTextOption opt;
    opt.setWrapMode(QTextOption::NoWrap);
    p.drawText(textRect, text(), opt);
}
