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
#ifndef ICONPROVIDER_H
#define ICONPROVIDER_H

#define _iconForUrl(url) IconProvider::iconFromImage(mApp->iconProvider()->iconForUrl(url))

#include <QObject>
#include <QImage>
#include <QUrl>
#include <QStyle>

#include "qz_namespace.h"

class QTimer;
class QIcon;

class WebView;

class QT_QUPZILLA_EXPORT IconProvider : public QObject
{
    Q_OBJECT
public:
    explicit IconProvider(QObject* parent = 0);
    ~IconProvider();

    void clearIconDatabase();

    void saveIcon(WebView* view);
    QImage iconForUrl(const QUrl &url);
    QImage iconForDomain(const QUrl &url);

    static QIcon iconFromImage(const QImage &image);

    static QIcon iconFromBase64(const QByteArray &data);
    static QByteArray iconToBase64(const QIcon &icon);

    static QIcon standardIcon(QStyle::StandardPixmap icon);
    static QIcon fromTheme(const QString &icon);

    static QIcon emptyWebIcon();
    static QImage emptyWebImage();

signals:

public slots:
    void saveIconsToDatabase();

private:
    QTimer* m_timer;

    struct Icon {
        QUrl url;
        QImage image;
    };

    static QImage m_emptyWebImage;
    QList<Icon> m_iconBuffer;

};

#endif // ICONPROVIDER_H
