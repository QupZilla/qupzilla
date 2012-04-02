/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
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
#include <QSize>
#include <QUrl>
#include <QWebPluginFactory>
#include <QPixmap>

#include "qz_namespace.h"

class QWebPage;

class QT_QUPZILLA_EXPORT CleanPluginFactory : public QWebPluginFactory
{
    Q_OBJECT
public:
    explicit CleanPluginFactory(QObject* parent = 0);

    QList<QWebPluginFactory::Plugin> plugins() const;
    QObject* create(const QString &mimeType, const QUrl &url, const QStringList &argumentNames, const QStringList &argumentValues) const;
};

class QT_QUPZILLA_EXPORT PageThumbnailer : public QObject
{
    Q_OBJECT
public:
    explicit PageThumbnailer(QObject* parent = 0);
    ~PageThumbnailer();

    void setSize(const QSize &size);

    void setUrl(const QUrl &url);
    QUrl url();

    bool loadTitle();
    void setLoadTitle(bool load);
    QString title();

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
    QString m_title;
    bool m_loadTitle;
};

#endif // PAGETHUMBNAILER_H
