#include "popupwebview.h"
#include "popupwebpage.h"
#include "mainapplication.h"
#include "iconprovider.h"

PopupWebView::PopupWebView(QWidget* parent)
    : WebView(parent)
    , m_page(0)
    , m_clickedFrame(0)
    , m_menu(new QMenu(this))
{
}

void PopupWebView::setWebPage(PopupWebPage* page)
{
    if (m_page == page) {
        return;
    }

    if (m_page) {
        delete m_page;
        m_page = 0;
    }

    m_page = page;
    m_page->setParent(this);
    setPage(m_page);

    // Set default zoom
    setZoom(mApp->defaultZoom());
}

PopupWebPage* PopupWebView::webPage()
{
    return m_page;
}

QWidget* PopupWebView::overlayForJsAlert()
{
    return this;
}

void PopupWebView::closeView()
{
    parentWidget()->close();
}

void PopupWebView::contextMenuEvent(QContextMenuEvent *event)
{
    m_menu->clear();
    m_clickedFrame = 0;

    QWebFrame* frameAtPos = page()->frameAt(event->pos());
    QWebHitTestResult r = page()->mainFrame()->hitTestContent(event->pos());

    if (!r.linkUrl().isEmpty() && r.linkUrl().scheme() != "javascript") {
        if (page()->selectedText() == r.linkText()) {
            findText("");
        }
        m_menu->addAction(tr("Open link in new &window"), this, SLOT(openUrlInNewWindow()))->setData(r.linkUrl());
        m_menu->addSeparator();
        m_menu->addAction(QIcon::fromTheme("document-save"), tr("&Save link as..."), this, SLOT(downloadLinkToDisk()))->setData(r.linkUrl());
        m_menu->addAction(QIcon::fromTheme("mail-message-new"), tr("Send link..."), this, SLOT(sendLinkByMail()))->setData(r.linkUrl());
        m_menu->addAction(QIcon::fromTheme("edit-copy"), tr("&Copy link address"), this, SLOT(copyLinkToClipboard()))->setData(r.linkUrl());
        m_menu->addSeparator();
        if (!selectedText().isEmpty()) {
            m_menu->addAction(pageAction(QWebPage::Copy));
        }
    }

    if (!r.imageUrl().isEmpty()) {
        if (!m_menu->isEmpty()) {
            m_menu->addSeparator();
        }
        m_menu->addAction(tr("Show i&mage"), this, SLOT(openActionUrl()))->setData(r.imageUrl());
        m_menu->addAction(tr("Copy im&age"), this, SLOT(copyImageToClipboard()))->setData(r.imageUrl());
        m_menu->addAction(QIcon::fromTheme("edit-copy"), tr("Copy image ad&dress"), this, SLOT(copyLinkToClipboard()))->setData(r.imageUrl());
        m_menu->addSeparator();
        m_menu->addAction(QIcon::fromTheme("document-save"), tr("&Save image as..."), this, SLOT(downloadLinkToDisk()))->setData(r.imageUrl());
        m_menu->addAction(QIcon::fromTheme("mail-message-new"), tr("Send image..."), this, SLOT(sendLinkByMail()))->setData(r.imageUrl());
        m_menu->addSeparator();
        //menu->addAction(tr("Block image"), this, SLOT(blockImage()))->setData(r.imageUrl().toString());
        if (!selectedText().isEmpty()) {
            m_menu->addAction(pageAction(QWebPage::Copy));
        }
    }

    QWebElement element = r.element();
    if (!element.isNull() && (element.tagName().toLower() == "input" || element.tagName().toLower() == "textarea" ||
                              element.tagName().toLower() == "video" || element.tagName().toLower() == "audio")) {
        if (m_menu->isEmpty()) {
            page()->createStandardContextMenu()->popup(QCursor::pos());
            return;
        }
    }

    if (m_menu->isEmpty() && selectedText().isEmpty()) {
        QAction* action = m_menu->addAction(tr("&Back"), this, SLOT(back()));
        action->setIcon(IconProvider::standardIcon(QStyle::SP_ArrowBack));
        action->setEnabled(history()->canGoBack());

        action = m_menu->addAction(tr("&Forward"), this, SLOT(forward()));
        action->setIcon(IconProvider::standardIcon(QStyle::SP_ArrowForward));
        action->setEnabled(history()->canGoForward());

        m_menu->addAction(IconProvider::standardIcon(QStyle::SP_BrowserReload), tr("&Reload"), this, SLOT(reload()));
        action = m_menu->addAction(IconProvider::standardIcon(QStyle::SP_BrowserStop), tr("S&top"), this, SLOT(stop()));
        action->setEnabled(isLoading());
        m_menu->addSeparator();

        if (frameAtPos && page()->mainFrame() != frameAtPos) {
            m_clickedFrame = frameAtPos;
            QMenu* menu = new QMenu(tr("This frame"));
            menu->addAction(tr("Show &only this frame"), this, SLOT(loadClickedFrame()));
            menu->addSeparator();
            menu->addAction(IconProvider::standardIcon(QStyle::SP_BrowserReload), tr("&Reload"), this, SLOT(reloadClickedFrame()));
            menu->addAction(QIcon::fromTheme("document-print"), tr("Print frame"), this, SLOT(printClickedFrame()));
            menu->addSeparator();
            menu->addAction(QIcon::fromTheme("zoom-in"), tr("Zoom &in"), this, SLOT(clickedFrameZoomIn()));
            menu->addAction(QIcon::fromTheme("zoom-out"), tr("&Zoom out"), this, SLOT(clickedFrameZoomOut()));
            menu->addAction(QIcon::fromTheme("zoom-original"), tr("Reset"), this, SLOT(clickedFrameZoomReset()));
            menu->addSeparator();
            menu->addAction(QIcon::fromTheme("text-html"), tr("Show so&urce of frame"), this, SLOT(showClickedFrameSource()));

            m_menu->addMenu(menu);
        }

        m_menu->addSeparator();
        m_menu->addAction(QIcon::fromTheme("document-save"), tr("&Save page as..."), this, SLOT(downloadLinkToDisk()))->setData(url());
        m_menu->addAction(QIcon::fromTheme("edit-copy"), tr("&Copy page link"), this, SLOT(copyLinkToClipboard()))->setData(url());
        m_menu->addAction(QIcon::fromTheme("mail-message-new"), tr("Send page link..."), this, SLOT(sendLinkByMail()))->setData(url());
        m_menu->addAction(QIcon::fromTheme("document-print"), tr("&Print page"), this, SLOT(printPage()));
        m_menu->addSeparator();
        m_menu->addAction(QIcon::fromTheme("edit-select-all"), tr("Select &all"), this, SLOT(selectAll()));
        m_menu->addSeparator();
        m_menu->addAction(QIcon::fromTheme("text-html"), tr("Show so&urce code"), this, SLOT(showSource()));
        m_menu->addSeparator();
        m_menu->addAction(QIcon::fromTheme("dialog-information"), tr("Show info ab&out site"), this, SLOT(showSiteInfo()));
    }

    if (!selectedText().isEmpty()) {
        QString selectedText = page()->selectedText();

        m_menu->addAction(pageAction(QWebPage::Copy));
        m_menu->addAction(QIcon::fromTheme("mail-message-new"), tr("Send text..."), this, SLOT(sendLinkByMail()))->setData(selectedText);
        m_menu->addSeparator();

        QString langCode = mApp->getActiveLanguage().left(2);
        QUrl googleTranslateUrl = QUrl(QString("http://translate.google.com/#auto|%1|%2").arg(langCode, selectedText));
        m_menu->addAction(QIcon(":icons/menu/translate.png"), tr("Google Translate"), this, SLOT(openActionUrl()))->setData(googleTranslateUrl);
        m_menu->addAction(tr("Dictionary"), this, SLOT(openActionUrl()))->setData("http://" + (langCode != "" ? langCode + "." : langCode) + "wiktionary.org/wiki/Special:Search?search=" + selectedText);
        m_menu->addSeparator();

        QString selectedString = selectedText.trimmed();
        if (!selectedString.contains(".")) {
            // Try to add .com
            selectedString.append(".com");
        }
        QUrl guessedUrl = QUrl::fromUserInput(selectedString);

        if (isUrlValid(guessedUrl)) {
            m_menu->addAction(QIcon(":/icons/menu/popup.png"), tr("Go to &web address"), this, SLOT(openActionUrl()))->setData(guessedUrl);
        }
    }

#if (QTWEBKIT_VERSION >= QTWEBKIT_VERSION_CHECK(2, 2, 0))
//    still bugged? in 4.8 RC (it shows selection of webkit's internal source, not html from page)
//    it may or may not be bug, but this implementation is useless for us
//
//    if (!selectedHtml().isEmpty())
//        menu->addAction(tr("Show source of selection"), this, SLOT(showSourceOfSelection()));
#endif

    if (!m_menu->isEmpty()) {
        //Prevent choosing first option with double rightclick
        QPoint pos = QCursor::pos();
        QPoint p(pos.x(), pos.y() + 1);
        m_menu->popup(p);
        return;
    }

    WebView::contextMenuEvent(event);
}
