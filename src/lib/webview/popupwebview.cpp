#include "popupwebview.h"
#include "popupwebpage.h"
#include "mainapplication.h"
#include "qupzilla.h"
#include "tabwidget.h"
#include "iconprovider.h"
#include "enhancedmenu.h"

#include <QWebFrame>

PopupWebView::PopupWebView(QWidget* parent)
    : WebView(parent)
    , m_page(0)
    , m_menu(new Menu(this))
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
}

PopupWebPage* PopupWebView::webPage()
{
    return m_page;
}

QWidget* PopupWebView::overlayForJsAlert()
{
    return this;
}

void PopupWebView::openUrlInNewTab(const QUrl &url, Qz::NewTabPositionFlag position)
{
    Q_UNUSED(position)

    QupZilla* window = mApp->getWindow();

    if (window) {
        window->tabWidget()->addView(url, Qz::NT_SelectedTab);
        window->raise();
    }
}

void PopupWebView::closeView()
{
    parentWidget()->close();
}

void PopupWebView::contextMenuEvent(QContextMenuEvent* event)
{
    m_menu->clear();

    const QWebHitTestResult &hitTest = page()->mainFrame()->hitTestContent(event->pos());

    createContextMenu(m_menu, hitTest, event->pos());

    if (!m_menu->isEmpty()) {
        //Prevent choosing first option with double rightclick
        const QPoint &pos = QCursor::pos();
        QPoint p(pos.x(), pos.y() + 1);
        m_menu->popup(p);
        return;
    }

    WebView::contextMenuEvent(event);
}
