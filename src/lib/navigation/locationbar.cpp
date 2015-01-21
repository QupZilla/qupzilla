/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
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
#include "browserwindow.h"
#include "tabbedwebview.h"
#include "mainapplication.h"
#include "webpage.h"
#include "tabwidget.h"
#include "bookmarksicon.h"
#include "bookmarks.h"
#include "bookmarkitem.h"
#include "siteicon.h"
#include "goicon.h"
#include "rssicon.h"
#include "downicon.h"
#include "qztools.h"
#include "iconprovider.h"
#include "qzsettings.h"
#include "colors.h"
#include "autofillicon.h"
#include "searchenginesmanager.h"
#include "completer/locationcompleter.h"

#include <QTimer>
#include <QMimeData>
#include <QCompleter>
#include <QStringListModel>
#include <QContextMenuEvent>
#include <QStyleOptionFrameV3>

LocationBar::LocationBar(BrowserWindow* window)
    : LineEdit(window)
    , m_window(window)
    , m_webView(0)
    , m_pasteAndGoAction(0)
    , m_clearAction(0)
    , m_holdingAlt(false)
    , m_oldTextLength(0)
    , m_currentTextLength(0)
    , m_loadProgress(0)
    , m_progressVisible(false)
{
    setObjectName("locationbar");
    setDragEnabled(true);

    // Disable Oxygen QLineEdit transitions, it breaks with setText() && home()
    setProperty("_kde_no_animations", QVariant(true));

    m_bookmarkIcon = new BookmarksIcon(this);
    m_goIcon = new GoIcon(this);
    m_rssIcon = new RssIcon(this);
    m_siteIcon = new SiteIcon(m_window, this);
    m_autofillIcon = new AutoFillIcon(this);
    DownIcon* down = new DownIcon(this);

    addWidget(m_siteIcon, LineEdit::LeftSide);
    addWidget(m_autofillIcon, LineEdit::RightSide);
    addWidget(m_bookmarkIcon, LineEdit::RightSide);
    addWidget(m_rssIcon, LineEdit::RightSide);
    addWidget(m_goIcon, LineEdit::RightSide);
    addWidget(down, LineEdit::RightSide);

    m_completer = new LocationCompleter(this);
    m_completer->setMainWindow(m_window);
    m_completer->setLocationBar(this);
    connect(m_completer, SIGNAL(showCompletion(QString)), this, SLOT(showCompletion(QString)));
    connect(m_completer, SIGNAL(showDomainCompletion(QString)), this, SLOT(showDomainCompletion(QString)));
    connect(m_completer, SIGNAL(loadCompletion()), this, SLOT(requestLoadUrl()));
    connect(m_completer, SIGNAL(clearCompletion()), this, SLOT(clearCompletion()));

    m_domainCompleterModel = new QStringListModel(this);
    QCompleter* domainCompleter = new QCompleter(this);
    domainCompleter->setCompletionMode(QCompleter::InlineCompletion);
    domainCompleter->setModel(m_domainCompleterModel);
    setCompleter(domainCompleter);

    connect(this, SIGNAL(textEdited(QString)), this, SLOT(textEdited(QString)));
    connect(m_goIcon, SIGNAL(clicked(QPoint)), this, SLOT(requestLoadUrl()));
    connect(down, SIGNAL(clicked(QPoint)), m_completer, SLOT(showMostVisited()));
    connect(mApp->searchEnginesManager(), SIGNAL(activeEngineChanged()), this, SLOT(updatePlaceHolderText()));
    connect(mApp->searchEnginesManager(), SIGNAL(defaultEngineChanged()), this, SLOT(updatePlaceHolderText()));
    connect(mApp, SIGNAL(settingsReloaded()), SLOT(loadSettings()));

    loadSettings();

    updateSiteIcon();

    // Hide icons by default
    m_goIcon->setVisible(qzSettings->alwaysShowGoIcon);
    m_rssIcon->hide();
    m_autofillIcon->hide();

    QTimer::singleShot(0, this, SLOT(updatePlaceHolderText()));
}

TabbedWebView* LocationBar::webView() const
{
    return m_webView;
}

void LocationBar::setWebView(TabbedWebView* view)
{
    m_webView = view;

    m_bookmarkIcon->setWebView(m_webView);
    m_rssIcon->setWebView(m_webView);
    m_siteIcon->setWebView(m_webView);
    m_autofillIcon->setWebView(m_webView);

    connect(m_webView, SIGNAL(loadStarted()), SLOT(loadStarted()));
    connect(m_webView, SIGNAL(loadProgress(int)), SLOT(loadProgress(int)));
    connect(m_webView, SIGNAL(loadFinished(bool)), SLOT(loadFinished()));
    connect(m_webView, SIGNAL(iconChanged()), this, SLOT(updateSiteIcon()));
    connect(m_webView, SIGNAL(urlChanged(QUrl)), this, SLOT(showUrl(QUrl)));
    connect(m_webView, SIGNAL(rssChanged(bool)), this, SLOT(setRssIconVisible(bool)));
    connect(m_webView, SIGNAL(privacyChanged(bool)), this, SLOT(setPrivacyState(bool)));
}

void LocationBar::setText(const QString &text)
{
    m_oldTextLength = text.length();
    m_currentTextLength = m_oldTextLength;

    LineEdit::setText(text);

    refreshTextFormat();
}

void LocationBar::updatePlaceHolderText()
{
    QString engineName = qzSettings->searchWithDefaultEngine ?
                         mApp->searchEnginesManager()->defaultEngine().name :
                         mApp->searchEnginesManager()->activeEngine().name;
    setPlaceholderText(tr("Enter URL address or search on %1").arg(engineName));
}

void LocationBar::showCompletion(const QString &completion)
{
    LineEdit::setText(completion);

    // Move cursor to the end
    end(false);
}

void LocationBar::clearCompletion()
{
    m_webView->setFocus();
    showUrl(m_webView->url());
}

void LocationBar::showDomainCompletion(const QString &completion)
{
    m_domainCompleterModel->setStringList(QStringList() << completion);

    // We need to manually force the completion because model is updated asynchronously
    // But only force completion when the user actually added new text
    if (m_oldTextLength < m_currentTextLength)
        completer()->complete();
}

LoadRequest LocationBar::createLoadRequest() const
{
    LoadRequest req;

    // Check for Search Engine shortcut
    int firstSpacePos = text().indexOf(QLatin1Char(' '));
    if (firstSpacePos != -1) {
        const QString shortcut = text().left(firstSpacePos);
        const QString searchedString = text().mid(firstSpacePos).trimmed();

        SearchEngine en = mApp->searchEnginesManager()->engineForShortcut(shortcut);
        if (!en.name.isEmpty()) {
            req = mApp->searchEnginesManager()->searchResult(en, searchedString);
        }
    }

    // Check for Bookmark keyword
    QList<BookmarkItem*> items = mApp->bookmarks()->searchKeyword(text());
    if (!items.isEmpty()) {
        BookmarkItem* item = items.first();
        item->updateVisitCount();
        req.setUrl(item->url());
    }

    if (req.isEmpty()) {
        const QUrl guessedUrl = WebView::guessUrlFromString(text());
        if (!guessedUrl.isEmpty())
            req.setUrl(guessedUrl);
        else
            req.setUrl(QUrl::fromEncoded(text().toUtf8()));
    }

    return req;
}

QString LocationBar::convertUrlToText(const QUrl &url)
{
    // It was most probably entered by user, so don't urlencode it
    if (url.scheme().isEmpty()) {
        return QUrl::fromPercentEncoding(url.toEncoded());
    }

    QString stringUrl = QzTools::urlEncodeQueryString(url);

    if (stringUrl == QLatin1String("qupzilla:speeddial") || stringUrl == QLatin1String("about:blank")) {
        stringUrl.clear();
    }

    return stringUrl;
}

void LocationBar::refreshTextFormat()
{
    if (!m_webView) {
        return;
    }

    TextFormat textFormat;
    const QString hostName = m_webView->url().isEmpty() ? QUrl(text()).host() : m_webView->url().host();

    if (!hostName.isEmpty()) {
        const int hostPos = text().indexOf(hostName);

        if (hostPos > 0) {
            QTextCharFormat format;
            format.setForeground(Colors::mid(palette().color(QPalette::Base), palette().color(QPalette::Text), 1, 1));

            QTextLayout::FormatRange schemePart;
            schemePart.start = 0;
            schemePart.length = hostPos;
            schemePart.format = format;

            QTextLayout::FormatRange hostPart;
            hostPart.start = hostPos;
            hostPart.length = hostName.size();

            QTextLayout::FormatRange remainingPart;
            remainingPart.start = hostPos + hostName.size();
            remainingPart.length = text().size() - remainingPart.start;
            remainingPart.format = format;

            textFormat.append(schemePart);
            textFormat.append(hostPart);
            textFormat.append(remainingPart);
        }
    }

    setTextFormat(textFormat);
}

void LocationBar::requestLoadUrl()
{
    const LoadRequest req = createLoadRequest();
    const QString urlString = convertUrlToText(req.url());

    m_completer->closePopup();
    m_webView->setFocus();

    if (urlString != text()) {
        setText(urlString);
    }

    m_webView->userLoadAction(req);
}

void LocationBar::textEdited(const QString &text)
{
    m_oldTextLength = m_currentTextLength;
    m_currentTextLength = text.length();

    if (!text.isEmpty()) {
        m_completer->complete(text);
    }
    else {
        m_completer->closePopup();
    }

    setGoIconVisible(true);
}

void LocationBar::setGoIconVisible(bool state)
{
    if (state) {
        m_bookmarkIcon->hide();
        m_rssIcon->hide();
        m_goIcon->show();
    }
    else {
        m_rssIcon->setVisible(m_webView && m_webView->hasRss());
        m_bookmarkIcon->show();

        if (!qzSettings->alwaysShowGoIcon) {
            m_goIcon->hide();
        }
    }

    updateTextMargins();
}

void LocationBar::setRssIconVisible(bool state)
{
    m_rssIcon->setVisible(state);

    updateTextMargins();
}

void LocationBar::showUrl(const QUrl &url)
{
    if (hasFocus() || url.isEmpty()) {
        return;
    }

    const QString stringUrl = convertUrlToText(url);

    if (text() == stringUrl) {
        home(false);
        refreshTextFormat();
        return;
    }

    // Set converted url as text
    setText(stringUrl);

    // Move cursor to the start
    home(false);

    m_bookmarkIcon->checkBookmark(url);
}

void LocationBar::updateSiteIcon()
{
    const QIcon icon = m_webView ? m_webView->icon() : IconProvider::emptyWebIcon();
    m_siteIcon->setIcon(QIcon(icon.pixmap(16, 16)));
}

void LocationBar::setPrivacyState(bool state)
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
    requestLoadUrl();
}

void LocationBar::contextMenuEvent(QContextMenuEvent* event)
{
    if (!m_pasteAndGoAction) {
        m_pasteAndGoAction = new QAction(QIcon::fromTheme("edit-paste"), tr("Paste And &Go"), this);
        m_pasteAndGoAction->setShortcut(QKeySequence("Ctrl+Shift+V"));
        connect(m_pasteAndGoAction, SIGNAL(triggered()), this, SLOT(pasteAndGo()));
    }

    QMenu* menu = createContextMenu(m_pasteAndGoAction);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    // Prevent choosing first option with double rightclick
    QPoint pos = event->globalPos();
    pos.setY(pos.y() + 1);
    menu->popup(pos);
}

void LocationBar::showEvent(QShowEvent* event)
{
    LineEdit::showEvent(event);

    refreshTextFormat();
}

void LocationBar::focusInEvent(QFocusEvent* event)
{
    if (m_webView) {
        const QString stringUrl = convertUrlToText(m_webView->url());

        // Text has been edited, let's show go button
        if (stringUrl != text()) {
            setGoIconVisible(true);
        }
    }

    clearTextFormat();
    LineEdit::focusInEvent(event);
}

void LocationBar::focusOutEvent(QFocusEvent* event)
{
    // Context menu or completer popup were opened
    // Let's block focusOutEvent to trick QLineEdit and paint cursor properly
    if (event->reason() == Qt::PopupFocusReason) {
        return;
    }

    LineEdit::focusOutEvent(event);

    setGoIconVisible(false);

    if (text().trimmed().isEmpty()) {
        clear();
    }

    refreshTextFormat();
}

void LocationBar::dropEvent(QDropEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        QUrl dropUrl = event->mimeData()->urls().at(0);
        if (WebView::isUrlValid(dropUrl)) {
            setText(dropUrl.toString());

            m_webView->setFocus();
            m_webView->userLoadAction(dropUrl);

            QFocusEvent event(QFocusEvent::FocusOut);
            LineEdit::focusOutEvent(&event);
            return;
        }
    }
    else if (event->mimeData()->hasText()) {
        QUrl dropUrl = QUrl(event->mimeData()->text().trimmed());
        if (WebView::isUrlValid(dropUrl)) {
            setText(dropUrl.toString());

            m_webView->setFocus();
            m_webView->userLoadAction(dropUrl);

            QFocusEvent event(QFocusEvent::FocusOut);
            LineEdit::focusOutEvent(&event);
            return;
        }

    }

    LineEdit::dropEvent(event);
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
        m_completer->complete(text());
        break;

    case Qt::Key_Left:
        m_completer->closePopup();
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
            requestLoadUrl();
            m_holdingAlt = false;
            break;

        case Qt::AltModifier:
            m_completer->closePopup();
            m_window->tabWidget()->addView(createLoadRequest());
            m_holdingAlt = false;
            break;

        default:
            requestLoadUrl();
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
        !text().endsWith(localDomain) && !text().endsWith(QLatin1Char('/'))
       ) {
        LineEdit::setText(text().append(localDomain));
    }

    LineEdit::keyReleaseEvent(event);
}

void LocationBar::loadStarted()
{
    m_progressVisible = true;
    m_autofillIcon->hide();
    m_siteIcon->setIcon(IconProvider::emptyWebIcon());
}

void LocationBar::loadProgress(int progress)
{
    if (qzSettings->showLoadingProgress) {
        m_loadProgress = progress;
        update();
    }
}

void LocationBar::loadFinished()
{
    if (qzSettings->showLoadingProgress) {
        QTimer::singleShot(700, this, SLOT(hideProgress()));
    }

    WebPage* page = qobject_cast<WebPage*>(m_webView->page());

    if (page && page->hasMultipleUsernames()) {
        m_autofillIcon->setFormData(page->autoFillData());
        m_autofillIcon->show();
    }

    updateSiteIcon();
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

void LocationBar::hideProgress()
{
    if (qzSettings->showLoadingProgress && m_loadProgress == 100) {
        m_progressVisible = false;
        update();
    }
}

void LocationBar::paintEvent(QPaintEvent* event)
{
    LineEdit::paintEvent(event);

    // Show loading progress
    if (qzSettings->showLoadingProgress && m_progressVisible) {
        QStyleOptionFrameV3 option;
        initStyleOption(&option);

        int lm, tm, rm, bm;
        getTextMargins(&lm, &tm, &rm, &bm);

        QRect contentsRect = style()->subElementRect(QStyle::SE_LineEditContents, &option, this);
        contentsRect.adjust(lm, tm, -rm, -bm);

        QColor bg = m_progressColor;
        if (!bg.isValid() || bg.alpha() == 0) {
            bg = Colors::mid(palette().color(QPalette::Base), palette().color(QPalette::Text), m_progressStyle > 0 ? 4 : 8, 1);
        }

        QPainter p(this);
        p.setBrush(QBrush(bg));

        // We are painting over text, make sure the text stays visible
        p.setOpacity(0.5);

        QPen outlinePen(bg.darker(110), 0.8);
        p.setPen(outlinePen);

        switch (m_progressStyle) {
        case ProgressFilled: {
            QRect bar = contentsRect.adjusted(0, 1, 0, -1);
            bar.setWidth(bar.width() * m_loadProgress / 100);
            const int roundness = bar.height() / 4.0;
            p.drawRoundedRect(bar, roundness, roundness);
            break;
        }
        case ProgressBottom: {
            outlinePen.setWidthF(0.3);
            outlinePen.setColor(outlinePen.color().darker(130));
            p.setPen(outlinePen);
            QRect bar(contentsRect.x(), contentsRect.bottom() - 3,
                      contentsRect.width() * m_loadProgress / 100.0, 3);
            p.drawRoundedRect(bar, 1, 1);
            break;
        }
        case ProgressTop: {
            outlinePen.setWidthF(0.3);
            outlinePen.setColor(outlinePen.color().darker(130));
            p.setPen(outlinePen);
            QRect bar(contentsRect.x(), contentsRect.top() + 1, contentsRect.width() * m_loadProgress / 100.0, 3);
            p.drawRoundedRect(bar, 1, 1);
            break;
        }
        default:
            break;
        }
    }
}
