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
#include "pagethumbnailer.h"

#include <QPainter>
#include <QApplication>
#include <QWebEngineView>

PageThumbnailer::PageThumbnailer(QObject* parent)
    : QObject(parent)
    , m_view(new QWebEngineView())
    , m_size(QSize(450, 253))
    , m_loadTitle(false)
{
    // Every page should fit in this resolution
    m_view->resize(QSize(1280, 720));

    // Well ...
    QWidget *w = QApplication::activeWindow();
    m_view->show();
    if (w) {
        QApplication::setActiveWindow(w);
        w->activateWindow();
    }
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
    m_view->load(m_url);

    connect(m_view, &QWebEngineView::loadFinished, this, &PageThumbnailer::createThumbnail);
}

void PageThumbnailer::createThumbnail(bool status)
{
    if (!status) {
        emit thumbnailCreated(QPixmap());
        return;
    }

    m_title = m_view->title().trimmed();

    emit thumbnailCreated(m_view->grab().scaled(m_size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

PageThumbnailer::~PageThumbnailer()
{
    m_view->deleteLater();
}
