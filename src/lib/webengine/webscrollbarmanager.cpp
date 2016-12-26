/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2016 David Rosca <nowrep@gmail.com>
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

#include "webscrollbarmanager.h"
#include "webscrollbar.h"
#include "webview.h"
#include "webpage.h"
#include "mainapplication.h"
#include "scripts.h"
#include "settings.h"

#include <QPaintEvent>
#include <QWebEngineProfile>
#include <QWebEngineScriptCollection>

Q_GLOBAL_STATIC(WebScrollBarManager, qz_web_scrollbar_manager)

class WebScrollBarCornerWidget : QWidget
{
public:
    explicit WebScrollBarCornerWidget(WebView *view)
        : QWidget()
        , m_view(view)
    {
    }

    void updateVisibility(bool visible, int thickness)
    {
        if (visible) {
            setParent(m_view->overlayWidget());
            resize(thickness, thickness);
            move(m_view->width() - width(), m_view->height() - height());
            show();
        } else {
            hide();
        }
    }

private:
    void paintEvent(QPaintEvent *ev) override
    {
        QPainter painter(this);
        painter.fillRect(ev->rect(), m_view->page()->backgroundColor());
        QWidget::paintEvent(ev);
    }

    WebView *m_view;
};

struct ScrollBarData {
    ~ScrollBarData() {
        delete vscrollbar;
        delete hscrollbar;
        delete corner;
    }

    WebScrollBar *vscrollbar;
    WebScrollBar *hscrollbar;
    WebScrollBarCornerWidget *corner;
};

WebScrollBarManager::WebScrollBarManager(QObject *parent)
    : QObject(parent)
{
    loadSettings();
}

void WebScrollBarManager::loadSettings()
{
    m_enabled = Settings().value(QSL("Web-Browser-Settings/UseNativeScrollbars"), true).toBool();

    if (!m_enabled) {
        for (WebView *view : m_scrollbars.keys()) {
            removeWebView(view);
        }
    }
}

void WebScrollBarManager::addWebView(WebView *view)
{
    if (!m_enabled) {
        return;
    }

    delete m_scrollbars.value(view);

    ScrollBarData *data = new ScrollBarData;
    data->vscrollbar = new WebScrollBar(Qt::Vertical, view);
    data->hscrollbar = new WebScrollBar(Qt::Horizontal, view);
    data->corner = new WebScrollBarCornerWidget(view);
    m_scrollbars[view] = data;

    auto updateValues = [=]() {
        const int thickness = data->vscrollbar->thickness();
        const QSize viewport = viewportSize(view, thickness);
        data->vscrollbar->updateValues(viewport);
        data->hscrollbar->updateValues(viewport);
        data->corner->updateVisibility(data->vscrollbar->isVisible() && data->hscrollbar->isVisible(), thickness);
    };

    connect(view, &WebView::viewportResized, this, updateValues);
    connect(view->page(), &WebPage::contentsSizeChanged, this, updateValues);
    connect(view->page(), &WebPage::scrollPositionChanged, this, updateValues);

    if (m_scrollbars.size() == 1) {
        createUserScript();
    }
}

void WebScrollBarManager::removeWebView(WebView *view)
{
    if (m_scrollbars.size() == 1) {
        removeUserScript();
    }

    disconnect(view, 0, this, 0);
    disconnect(view->page(), 0, this, 0);

    delete m_scrollbars.take(view);
}

QScrollBar *WebScrollBarManager::scrollBar(Qt::Orientation orientation, WebView *view) const
{
    ScrollBarData *d = m_scrollbars.value(view);
    if (!d) {
        return nullptr;
    }
    return orientation == Qt::Vertical ? d->vscrollbar : d->hscrollbar;
}

WebScrollBarManager *WebScrollBarManager::instance()
{
    return qz_web_scrollbar_manager();
}

void WebScrollBarManager::createUserScript()
{
    Q_ASSERT(!m_scrollbars.isEmpty());

    const int thickness = (*m_scrollbars.begin())->vscrollbar->thickness();

    QWebEngineScript script;
    script.setName(QSL("_qupzilla_scrollbar"));
    script.setInjectionPoint(QWebEngineScript::DocumentReady);
    script.setWorldId(WebPage::SafeJsWorld);
    script.setSourceCode(Scripts::setCss(QSL("body::-webkit-scrollbar{width:%1px;height:%1px;}").arg(thickness)));
    mApp->webProfile()->scripts()->insert(script);
}

void WebScrollBarManager::removeUserScript()
{
    QWebEngineScript script = mApp->webProfile()->scripts()->findScript(QSL("_qupzilla_scrollbar"));
    mApp->webProfile()->scripts()->remove(script);
}

QSize WebScrollBarManager::viewportSize(WebView *view, int thickness) const
{
    QSize viewport = view->size();
    const QSize content = view->page()->contentsSize().toSize();

    // Check both axis
    if (content.width() - viewport.width() > 0) {
        viewport.setHeight(viewport.height() - thickness);
    }

    if (content.height() - viewport.height() > 0) {
        viewport.setWidth(viewport.width() - thickness);
    }

    // Check again against adjusted size
    if (viewport.height() == view->height() && content.width() - viewport.width() > 0) {
        viewport.setHeight(viewport.height() - thickness);
    }

    if (viewport.width() == view->width() && content.height() - viewport.height() > 0) {
        viewport.setWidth(viewport.width() - thickness);
    }

    return viewport;
}
