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
#include "pagethumbnailer.h"
#include "scripts.h"
#include "webview.h"

#include <QTimer>
#include <QApplication>

#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>

PageThumbnailer::PageThumbnailer(QObject* parent)
    : QObject(parent)
    , m_view(new QQuickWidget())
    , m_size(QSize(450, 253) * qApp->devicePixelRatio())
    , m_loadTitle(false)
{
    m_view->setAttribute(Qt::WA_DontShowOnScreen);
    m_view->setSource(QUrl(QSL("qrc:data/thumbnailer.qml")));
    m_view->rootContext()->setContextProperty(QSL("thumbnailer"), this);
    m_view->show();
}

void PageThumbnailer::setSize(const QSize &size)
{
    if (size.isValid()) {
        m_size = size;
    }
}

void PageThumbnailer::setUrl(const QUrl &url)
{
    if (url.isValid()) {
        m_url = url;
    }
}

QUrl PageThumbnailer::url()
{
    return m_url;
}

bool PageThumbnailer::loadTitle()
{
    return m_loadTitle;
}

void PageThumbnailer::setLoadTitle(bool load)
{
    m_loadTitle = load;
}

QString PageThumbnailer::title()
{
    QString title = m_title.isEmpty() ? m_url.host() : m_title;
    if (title.isEmpty()) {
        title = m_url.toString();
    }
    return title;
}

void PageThumbnailer::start()
{
    if (m_view->rootObject() && WebView::isUrlValid(m_url)) {
        m_view->rootObject()->setProperty("url", m_url);
    } else {
        QTimer::singleShot(500, this, [this]() {
            emit thumbnailCreated(QPixmap());
        });
    }
}

QString PageThumbnailer::afterLoadScript() const
{
    return Scripts::setCss(QSL("::-webkit-scrollbar{display:none;}"));
}

void PageThumbnailer::createThumbnail(bool status)
{
    if (!status) {
        emit thumbnailCreated(QPixmap());
        return;
    }

    QTimer::singleShot(1000, this, [this]() {
        m_title = m_view->rootObject()->property("title").toString().trimmed();
        emit thumbnailCreated(QPixmap::fromImage(m_view->grabFramebuffer().scaled(m_size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    });
}

PageThumbnailer::~PageThumbnailer()
{
    m_view->deleteLater();
}
