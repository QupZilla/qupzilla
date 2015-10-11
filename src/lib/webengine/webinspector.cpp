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
#include "webinspector.h"
#include "mainapplication.h"
#include "networkmanager.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkReply>

QList<QWebEngineView*> WebInspector::s_views;

WebInspector::WebInspector(QWidget *parent)
    : QWebEngineView(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName(QSL("web-inspector"));
    setMinimumHeight(80);

    registerView(this);

    connect(page(), &QWebEnginePage::windowCloseRequested, this, &WebInspector::deleteLater);
    connect(page(), &QWebEnginePage::loadFinished, this, &WebInspector::updateCloseButton);
}

WebInspector::~WebInspector()
{
    unregisterView(this);
}

void WebInspector::setView(QWebEngineView *view)
{
    QUrl inspectorUrl = QUrl(QSL("http://localhost:%1").arg(WEBINSPECTOR_PORT));
    int index = s_views.indexOf(view);

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

void WebInspector::updateCloseButton()
{
    page()->runJavaScript(QL1S("var toolbar = document.getElementsByClassName('inspector-view-toolbar')[1];"
                               "var button = document.createElement('button');"
                               "button.style.width = '22px';"
                               "button.style.height = '22px';"
                               "button.style.border = 'none';"
                               "button.style.cursor = 'pointer';"
                               "button.style.background = 'url(qrc:html/close.png) no-repeat';"
                               "button.style.backgroundPosition = 'center center';"
                               "button.addEventListener('click', function() {"
                               "    window.close();"
                               "});"
                               "toolbar.appendChild(button);"));
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
