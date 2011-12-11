/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca
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
#ifndef PAGETHUMBNAILER_H
#define PAGETHUMBNAILER_H

#include <QObject>
#include <QWebPage>
#include <QWebFrame>
#include <QPainter>
#include <QWebPluginFactory>

class CleanPluginFactory : public QWebPluginFactory
{
    Q_OBJECT
public:
    explicit CleanPluginFactory(QObject* parent = 0);

    QList<QWebPluginFactory::Plugin> plugins() const;
    QObject* create(const QString &mimeType, const QUrl &url, const QStringList &argumentNames, const QStringList &argumentValues) const;
};

class PageThumbnailer : public QObject
{
    Q_OBJECT
public:
    explicit PageThumbnailer(QObject* parent = 0);
    ~PageThumbnailer();

    void setSize(const QSize &size);

    void setUrl(const QUrl &url);
    QUrl url();

    void setEnableFlash(bool enable);

    void start();

signals:
    void thumbnailCreated(QPixmap);

public slots:

private slots:
    void createThumbnail(bool status);

private:
    QWebPage* m_page;

    QSize m_size;
    QUrl m_url;
};

#endif // PAGETHUMBNAILER_H
