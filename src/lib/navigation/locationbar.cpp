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
#include "locationbar.h"
#include "browserwindow.h"
#include "tabbedwebview.h"
#include "mainapplication.h"
#include "webpage.h"
#include "tabwidget.h"
#include "bookmarksicon.h"
#include "bookmarks.h"
#include "bookmarkitem.h"
#include "bookmarkstoolbar.h"
#include "siteicon.h"
#include "goicon.h"
#include "downicon.h"
#include "qztools.h"
#include "iconprovider.h"
#include "qzsettings.h"
#include "colors.h"
#include "autofillicon.h"
#include "completer/locationcompleter.h"

#include <QTimer>
#include <QMimeData>
#include <QCompleter>
#include <QStringListModel>
#include <QContextMenuEvent>
#include <QStyleOptionFrameV3>

LocationBar::LocationBar(QWidget *parent)
    : LineEdit(parent)
    , m_window(nullptr)
    , m_webView(0)
    , m_holdingAlt(false)
    , m_oldTextLength(0)
    , m_currentTextLength(0)
    , m_loadProgress(0)
    , m_progressVisible(false)
{
    setObjectName("locationbar");
    setDragEnabled(true);

    // Disable KDE QLineEdit transitions, it breaks with setText() && home()
    setProperty("_kde_no_animations", QVariant(true));

    m_bookmarkIcon = new BookmarksIcon(this);
    m_goIcon = new GoIcon(this);
    m_siteIcon = new SiteIcon(this);
    m_autofillIcon = new AutoFillIcon(this);
    DownIcon* down = new DownIcon(this);

    addWidget(m_siteIcon, LineEdit::LeftSide);
    addWidget(m_autofillIcon, LineEdit::RightSide);
    addWidget(m_bookmarkIcon, LineEdit::RightSide);
    addWidget(m_goIcon, LineEdit::RightSide);
    addWidget(down, LineEdit::RightSide);

    m_completer = new LocationCompleter(this);
    m_completer->setLocationBar(this);
    connect(m_completer, SIGNAL(showCompletion(QString,bool)), this, SLOT(showCompletion(QString,bool)));
    connect(m_completer, SIGNAL(showDomainCompletion(QString)), this, SLOT(showDomainCompletion(QString)));
    connect(m_completer, SIGNAL(clearCompletion()), this, SLOT(clearCompletion()));
    connect(m_completer, &LocationCompleter::loadRequested, this, &LocationBar::loadRequest);
    connect(m_completer, &LocationCompleter::popupClosed, this, &LocationBar::updateSiteIcon);

    m_domainCompleterModel = new QStringListModel(this);
    QCompleter* domainCompleter = new QCompleter(this);
    domainCompleter->setCompletionMode(QCompleter::InlineCompletion);
    domainCompleter->setModel(m_domainCompleterModel);
    setCompleter(domainCompleter);

    m_progressTimer = new QTimer(this);
    m_progressTimer->setInterval(700);
    m_progressTimer->setSingleShot(true);
    connect(m_progressTimer, &QTimer::timeout, this, &LocationBar::hideProgress);

    editAction(PasteAndGo)->setText(tr("Paste And &Go"));
    editAction(PasteAndGo)->setIcon(QIcon::fromTheme(QSL("edit-paste")));
    connect(editAction(PasteAndGo), SIGNAL(triggered()), this, SLOT(pasteAndGo()));

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
    m_autofillIcon->hide();

    QTimer::singleShot(0, this, SLOT(updatePlaceHolderText()));
}

BrowserWindow *LocationBar::browserWindow() const
{
    return m_window;
}

void LocationBar::setBrowserWindow(BrowserWindow *window)
{
    m_window = window;
    m_completer->setMainWindow(m_window);
    m_siteIcon->setBrowserWindow(m_window);
}

TabbedWebView* LocationBar::webView() const
{
    return m_webView;
}

void LocationBar::setWebView(TabbedWebView* view)
{
    m_webView = view;

    m_bookmarkIcon->setWebView(m_webView);
    m_siteIcon->setWebView(m_webView);
    m_autofillIcon->setWebView(m_webView);

    connect(m_webView, SIGNAL(loadStarted()), SLOT(loadStarted()));
    connect(m_webView, SIGNAL(loadProgress(int)), SLOT(loadProgress(int)));
    connect(m_webView, SIGNAL(loadFinished(bool)), SLOT(loadFinished()));
    connect(m_webView, SIGNAL(urlChanged(QUrl)), this, SLOT(showUrl(QUrl)));
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
    if (qzSettings->searchFromAddressBar) {
        setPlaceholderText(tr("Enter address or search with %1").arg(searchEngine().name));
    } else
        setPlaceholderText(tr("Enter address"));
}

void LocationBar::showCompletion(const QString &completion, bool completeDomain)
{
    LineEdit::setText(completion);

    // Move cursor to the end
    end(false);

    if (completeDomain) {
        completer()->complete();
    }

    updateSiteIcon();
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

QString LocationBar::convertUrlToText(const QUrl &url)
{
    // It was most probably entered by user, so don't urlencode it
    // Also don't urlencode JavaScript code
    if (url.scheme().isEmpty() || url.scheme() == QL1S("javascript")) {
        return QUrl::fromPercentEncoding(url.toEncoded());
    }

    QString stringUrl = QzTools::urlEncodeQueryString(url);

    if (stringUrl == QL1S("qupzilla:speeddial") || stringUrl == QL1S("about:blank")) {
        stringUrl.clear();
    }

    return stringUrl;
}

SearchEnginesManager::Engine LocationBar::searchEngine()
{
    if (!qzSettings->searchFromAddressBar) {
        return SearchEngine();
    } else if (qzSettings->searchWithDefaultEngine) {
        return mApp->searchEnginesManager()->defaultEngine();
    } else {
        return mApp->searchEnginesManager()->activeEngine();
    }
}

LocationBar::LoadAction LocationBar::loadAction(const QString &text)
{
    LoadAction action;

    const QString &t = text.trimmed();

    if (t.isEmpty()) {
        return action;
    }

    // Check for Search Engine shortcut
    const int firstSpacePos = t.indexOf(QLatin1Char(' '));
    if (qzSettings->searchFromAddressBar && firstSpacePos != -1) {
        const QString shortcut = t.left(firstSpacePos);
        const QString searchedString = t.mid(firstSpacePos).trimmed();

        SearchEngine en = mApp->searchEnginesManager()->engineForShortcut(shortcut);
        if (en.isValid()) {
            action.type = LoadAction::Search;
            action.searchEngine = en;
            action.loadRequest = mApp->searchEnginesManager()->searchResult(en, searchedString);
            return action;
        }
    }

    // Check for Bookmark keyword
    const QList<BookmarkItem*> items = mApp->bookmarks()->searchKeyword(t);
    if (!items.isEmpty()) {
        BookmarkItem* item = items.at(0);
        action.type = LoadAction::Bookmark;
        action.bookmark = item;
        action.loadRequest.setUrl(item->url());
        return action;
    }

    if (!qzSettings->searchFromAddressBar) {
        const QUrl &guessedUrl = QUrl::fromUserInput(t);
        if (guessedUrl.isValid()) {
            action.type = LoadAction::Url;
            action.loadRequest = guessedUrl;
        }
        return action;
    }

    // Check for one word search
    if (t != QL1S("localhost")
            && !QzTools::containsSpace(t)
            && !t.contains(QL1C('.'))
            && !t.contains(QL1C(':'))
            && !t.contains(QL1C('/'))
       ) {
        action.type = LoadAction::Search;
        action.searchEngine = searchEngine();
        action.loadRequest = mApp->searchEnginesManager()->searchResult(searchEngine(), t);
        return action;
    }

    // Otherwise load as url
    const QUrl &guessedUrl = QUrl::fromUserInput(t);
    if (guessedUrl.isValid()) {
        // Always allow javascript: to be loaded
        const bool forceLoad = guessedUrl.scheme() == QL1S("javascript");
        // Only allow spaces in query
        if (forceLoad || !QzTools::containsSpace(guessedUrl.toString(QUrl::RemoveQuery))) {
            // Only allow whitelisted schemes
            static const QSet<QString> whitelistedSchemes = {
                QSL("http"), QSL("https"), QSL("ftp"), QSL("file"),
                QSL("data"), QSL("about"), QSL("qupzilla")
            };
            if (forceLoad || whitelistedSchemes.contains(guessedUrl.scheme())) {
                action.type = LoadAction::Url;
                action.loadRequest = guessedUrl;
                return action;
            }
        }
    }

    // Search when creating url failed
    action.type = LoadAction::Search;
    action.searchEngine = searchEngine();
    action.loadRequest = mApp->searchEnginesManager()->searchResult(searchEngine(), t);
    return action;
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
    loadRequest(loadAction(text()).loadRequest);
}

void LocationBar::textEdited(const QString &text)
{
    m_oldTextLength = m_currentTextLength;
    m_currentTextLength = text.length();

    if (!text.isEmpty()) {
        m_completer->complete(text);
        m_siteIcon->setIcon(QIcon::fromTheme(QSL("edit-find"), QIcon(QSL(":icons/menu/search-icon.svg"))));
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
        m_goIcon->show();
    }
    else {
        m_bookmarkIcon->show();

        if (!qzSettings->alwaysShowGoIcon) {
            m_goIcon->hide();
        }
    }

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

void LocationBar::loadRequest(const LoadRequest &request)
{
    if (!m_webView->webTab()->isRestored()) {
        return;
    }

    const QString urlString = convertUrlToText(request.url());

    m_completer->closePopup();
    m_webView->setFocus();

    if (urlString != text()) {
        setText(urlString);
    }

    m_webView->userLoadAction(request);
}

void LocationBar::updateSiteIcon()
{
    if (m_completer->isVisible()) {
        m_siteIcon->setIcon(QIcon::fromTheme(QSL("edit-find"), QIcon(QSL(":icons/menu/search-icon.svg"))));
    } else {
        QIcon icon = IconProvider::emptyWebIcon();
        if (property("secured").toBool()) {
            icon = QIcon::fromTheme(QSL("document-encrypted"), icon);
        }
        m_siteIcon->setIcon(QIcon(icon.pixmap(16)));
    }
}

void LocationBar::setPrivacyState(bool state)
{
    m_siteIcon->setProperty("secured", QVariant(state));
    m_siteIcon->style()->unpolish(m_siteIcon);
    m_siteIcon->style()->polish(m_siteIcon);

    setProperty("secured", QVariant(state));
    style()->unpolish(this);
    style()->polish(this);

    updateSiteIcon();
}

void LocationBar::pasteAndGo()
{
    clear();
    paste();
    requestLoadUrl();
}

void LocationBar::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu* menu = createContextMenu();
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

    if (m_window && Settings().value("Browser-View-Settings/instantBookmarksToolbar").toBool()) {
        m_window->bookmarksToolbar()->show();
    }
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

    if (m_window && Settings().value("Browser-View-Settings/instantBookmarksToolbar").toBool()) {
        m_window->bookmarksToolbar()->hide();
    }
}

void LocationBar::dropEvent(QDropEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        const QUrl dropUrl = event->mimeData()->urls().at(0);
        if (WebView::isUrlValid(dropUrl)) {
            setText(dropUrl.toString());
            loadRequest(dropUrl);

            QFocusEvent event(QFocusEvent::FocusOut);
            LineEdit::focusOutEvent(&event);
            return;
        }
    }
    else if (event->mimeData()->hasText()) {
        const QString dropText = event->mimeData()->text().trimmed();
        const QUrl dropUrl = QUrl(dropText);
        if (WebView::isUrlValid(dropUrl)) {
            setText(dropUrl.toString());
            loadRequest(dropUrl);

            QFocusEvent event(QFocusEvent::FocusOut);
            LineEdit::focusOutEvent(&event);
            return;
        } else {
            setText(dropText);
            setFocus();
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
            if (!text().endsWith(QL1S(".com")))
                setText(text().append(QL1S(".com")));
            requestLoadUrl();
            m_holdingAlt = false;
            break;

        case Qt::AltModifier:
            m_completer->closePopup();
            if (m_window) {
                m_window->tabWidget()->addView(loadAction(text()).loadRequest);
            }
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

void LocationBar::loadStarted()
{
    m_progressVisible = true;
    m_progressTimer->stop();
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
        m_progressTimer->start();
    }

    WebPage* page = qobject_cast<WebPage*>(m_webView->page());

    if (page && !page->autoFillUsernames().isEmpty()) {
        m_autofillIcon->setUsernames(page->autoFillUsernames());
        m_autofillIcon->show();
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

void LocationBar::hideProgress()
{
    if (qzSettings->showLoadingProgress) {
        m_progressVisible = false;
        update();
    }
}

void LocationBar::paintEvent(QPaintEvent* event)
{
    LineEdit::paintEvent(event);

    // Show loading progress
    if (qzSettings->showLoadingProgress && m_progressVisible) {
        QStyleOptionFrame option;
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
