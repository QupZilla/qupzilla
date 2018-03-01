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
#include "webinspector.h"
#include "mainapplication.h"
#include "networkmanager.h"
#include "settings.h"
#include "webview.h"
#include "webpage.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QWebEngineSettings>

QList<QWebEngineView*> WebInspector::s_views;

WebInspector::WebInspector(QWidget *parent)
    : QWebEngineView(parent)
    , m_view(Q_NULLPTR)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName(QSL("web-inspector"));
    setMinimumHeight(80);

    m_height = Settings().value(QSL("Web-Inspector/height"), 80).toInt();
    m_windowSize = Settings().value(QSL("Web-Inspector/windowSize"), QSize(640, 480)).toSize();

    registerView(this);

    connect(page(), &QWebEnginePage::windowCloseRequested, this, &WebInspector::deleteLater);
    connect(page(), &QWebEnginePage::loadFinished, this, &WebInspector::loadFinished);
}

WebInspector::~WebInspector()
{
    if (m_view && hasFocus()) {
        m_view->setFocus();
    }

    unregisterView(this);

    if (isWindow()) {
        Settings().setValue(QSL("Web-Inspector/windowSize"), size());
    } else {
        Settings().setValue(QSL("Web-Inspector/height"), height());
    }
}

void WebInspector::setView(WebView *view)
{
    m_view = view;
    Q_ASSERT(isEnabled());

#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    page()->setInspectedPage(m_view->page());
    connect(m_view, &WebView::pageChanged, this, &WebInspector::deleteLater);
#else
    int port = qEnvironmentVariableIntValue("QTWEBENGINE_REMOTE_DEBUGGING");
    QUrl inspectorUrl = QUrl(QSL("http://localhost:%1").arg(port));
    int index = s_views.indexOf(m_view);

    QNetworkReply *reply = mApp->networkManager()->get(QNetworkRequest(inspectorUrl.resolved(QUrl("json/list"))));
    connect(reply, &QNetworkReply::finished, this, [=]() {
        QJsonArray clients = QJsonDocument::fromJson(reply->readAll()).array();
        QUrl pageUrl;
        if (clients.size() > index) {
            QJsonObject object = clients.at(index).toObject();
            pageUrl = inspectorUrl.resolved(QUrl(object.value(QSL("devtoolsFrontendUrl")).toString()));
        }
        load(pageUrl);
        pushView(this);
        show();
    });
#endif
}

void WebInspector::inspectElement()
{
    m_inspectElement = true;
}

bool WebInspector::isEnabled()
{
#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
    if (!qEnvironmentVariableIsSet("QTWEBENGINE_REMOTE_DEBUGGING")) {
        return false;
    }
#endif
    if (!mApp->webSettings()->testAttribute(QWebEngineSettings::JavascriptEnabled)) {
        return false;
    }
    return true;
}

void WebInspector::pushView(QWebEngineView *view)
{
    s_views.removeOne(view);
    s_views.prepend(view);
}

void WebInspector::registerView(QWebEngineView *view)
{
    s_views.prepend(view);
}

void WebInspector::unregisterView(QWebEngineView *view)
{
    s_views.removeOne(view);
}

void WebInspector::loadFinished()
{
    // Show close button only when docked
    if (!isWindow()) {
        page()->runJavaScript(QL1S("var button = Components.dockController._closeButton;"
                                   "button.setVisible(true);"
                                   "button.element.onmouseup = function() {"
                                   "    window.close();"
                                   "};"));
    }

    // Inspect element
    if (m_inspectElement) {
        m_view->triggerPageAction(QWebEnginePage::InspectElement);
        m_inspectElement = false;
    }
}

QSize WebInspector::sizeHint() const
{
    if (isWindow()) {
        return m_windowSize;
    }
    QSize s = QWebEngineView::sizeHint();
    s.setHeight(m_height);
    return s;
}

void WebInspector::keyPressEvent(QKeyEvent *event)
{
    Q_UNUSED(event)
    // Stop propagation
}

void WebInspector::keyReleaseEvent(QKeyEvent *event)
{
    Q_UNUSED(event)
    // Stop propagation
}
