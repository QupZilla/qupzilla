/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2016-2017 David Rosca <nowrep@gmail.com>
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

#include <QPointer>
#include <QPaintEvent>
#include <QWebEngineProfile>
#include <QWebEngineScriptCollection>
#include <QStyle>
#include <QStyleOption>

Q_GLOBAL_STATIC(WebScrollBarManager, qz_web_scrollbar_manager)

class WebScrollBarCornerWidget : public QWidget
{
public:
    explicit WebScrollBarCornerWidget(WebView *view)
        : QWidget()
        , m_view(view)
    {
        setAutoFillBackground(true);
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
        Q_UNUSED(ev)

        QStyleOption option;
        option.initFrom(this);
        option.rect = rect();

        QPainter p(this);
        if (mApp->styleName() == QL1S("breeze")) {
            p.fillRect(ev->rect(), option.palette.background());
        } else {
            style()->drawPrimitive(QStyle::PE_PanelScrollAreaCorner, &option, &p, this);
        }
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
    bool vscrollbarVisible = false;
    bool hscrollbarVisible = false;
    WebScrollBarCornerWidget *corner;
};

WebScrollBarManager::WebScrollBarManager(QObject *parent)
    : QObject(parent)
{
    m_scrollbarJs = QL1S("(function() {"
                         "var head = document.getElementsByTagName('head')[0];"
                         "if (!head) return;"
                         "var css = document.createElement('style');"
                         "css.setAttribute('type', 'text/css');"
                         "var size = %1 / window.devicePixelRatio + 'px';"
                         "css.appendChild(document.createTextNode('"
                         "   body::-webkit-scrollbar{width:'+size+';height:'+size+';}"
                         "'));"
                         "head.appendChild(css);"
                         "})()");

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

    const int thickness = data->vscrollbar->thickness();

    auto updateValues = [=]() {
        const QSize viewport = viewportSize(view, thickness);
        data->vscrollbar->updateValues(viewport);
        data->vscrollbar->setVisible(data->vscrollbarVisible);
        data->hscrollbar->updateValues(viewport);
        data->hscrollbar->setVisible(data->hscrollbarVisible);
        data->corner->updateVisibility(data->vscrollbarVisible && data->hscrollbarVisible, thickness);
    };

    connect(view, &WebView::viewportResized, data->vscrollbar, updateValues);
    connect(view->page(), &WebPage::scrollPositionChanged, data->vscrollbar, updateValues);

    connect(view->page(), &WebPage::contentsSizeChanged, data->vscrollbar, [=]() {
        const QString source = QL1S("var out = {"
                                    "vertical: document.documentElement && window.innerWidth > document.documentElement.clientWidth,"
                                    "horizontal: document.documentElement && window.innerHeight > document.documentElement.clientHeight"
                                    "};out;");

        QPointer<WebView> p(view);
        view->page()->runJavaScript(source, WebPage::SafeJsWorld, [=](const QVariant &res) {
            if (!p || !m_scrollbars.contains(view)) {
                return;
            }
            const QVariantMap map = res.toMap();
            data->vscrollbarVisible = map.value(QSL("vertical")).toBool();
            data->hscrollbarVisible = map.value(QSL("horizontal")).toBool();
            updateValues();
        });
    });

    connect(view, &WebView::zoomLevelChanged, data->vscrollbar, [=]() {
        view->page()->runJavaScript(m_scrollbarJs.arg(thickness));
    });

    if (m_scrollbars.size() == 1) {
        createUserScript(thickness);
    }
}

void WebScrollBarManager::removeWebView(WebView *view)
{
    if (!m_scrollbars.contains(view)) {
        return;
    }

    if (m_scrollbars.size() == 1) {
        removeUserScript();
    }

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

void WebScrollBarManager::createUserScript(int thickness)
{
    QWebEngineScript script;
    script.setName(QSL("_qupzilla_scrollbar"));
    script.setInjectionPoint(QWebEngineScript::DocumentReady);
    script.setWorldId(WebPage::SafeJsWorld);
    script.setSourceCode(m_scrollbarJs.arg(thickness));
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

    thickness /= view->devicePixelRatioF();

    ScrollBarData *data = m_scrollbars.value(view);
    Q_ASSERT(data);

    if (data->vscrollbarVisible) {
        viewport.setWidth(viewport.width() - thickness);
    }
    if (data->hscrollbarVisible) {
        viewport.setHeight(viewport.height() - thickness);
    }

#if 0
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
#endif

    return viewport;
}
