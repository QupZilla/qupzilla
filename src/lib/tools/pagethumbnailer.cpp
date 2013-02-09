/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2013  David Rosca <nowrep@gmail.com>
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
#include "mainapplication.h"
#include "networkmanagerproxy.h"

#include <QWebPage>
#include <QWebFrame>
#include <QPainter>

CleanPluginFactory::CleanPluginFactory(QObject* parent)
    : QWebPluginFactory(parent)
{
}

QList<QWebPluginFactory::Plugin> CleanPluginFactory::plugins() const
{
    return QList<QWebPluginFactory::Plugin>();
}

QObject* CleanPluginFactory::create(const QString &mimeType, const QUrl &url, const QStringList &argumentNames, const QStringList &argumentValues) const
{
    Q_UNUSED(mimeType)
    Q_UNUSED(url)
    Q_UNUSED(argumentNames)
    Q_UNUSED(argumentValues)

    return new QObject;
}

PageThumbnailer::PageThumbnailer(QObject* parent)
    : QObject(parent)
    , m_page(new QWebPage(this))
    , m_size(QSize(231, 130))
    , m_loadTitle(false)
{
    NetworkManagerProxy* networkProxy = new NetworkManagerProxy(this);
    networkProxy->setPrimaryNetworkAccessManager(mApp->networkManager());
    m_page->setNetworkAccessManager(networkProxy);

    m_page->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    m_page->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);

    // HD Ready -,-
    // Every page should fit in this resolution
    m_page->setViewportSize(QSize(1280, 720));
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

void PageThumbnailer::setEnableFlash(bool enable)
{
    if (!enable) {
        m_page->setPluginFactory(new CleanPluginFactory);
    }
}

void PageThumbnailer::start()
{
    m_page->mainFrame()->load(m_url);

    connect(m_page, SIGNAL(loadFinished(bool)), this, SLOT(createThumbnail(bool)));
}

void PageThumbnailer::createThumbnail(bool status)
{
    if (!status) {
        emit thumbnailCreated(QPixmap());
        return;
    }

    m_title = m_page->mainFrame()->title().trimmed();

    QPixmap pixmap(2 * m_size);

    qreal scalingFactor = 2 * static_cast<qreal>(m_size.width()) / 1280;

    QPainter painter(&pixmap);
    painter.scale(scalingFactor, scalingFactor);
    m_page->mainFrame()->render(&painter, QWebFrame::ContentsLayer);
    painter.end();

    emit thumbnailCreated(pixmap.scaled(m_size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

PageThumbnailer::~PageThumbnailer()
{
    m_page->deleteLater();
}
