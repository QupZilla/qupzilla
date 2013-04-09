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
#ifndef ICONPROVIDER_H
#define ICONPROVIDER_H

#define qIconProvider IconProvider::instance()
#define _iconForUrl(url) qIconProvider->iconFromImage(qIconProvider->iconForUrl(url))

#include <QWidget>
#include <QImage>
#include <QUrl>
#include <QStyle>

#include "qz_namespace.h"

class QTimer;
class QIcon;

class WebView;

// Needs to be QWidget subclass, otherwise qproperty- setting won't work
class QT_QUPZILLA_EXPORT IconProvider : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QPixmap bookmarkIcon READ bookmarkIcon WRITE setBookmarkIcon)

public:
    explicit IconProvider(QWidget* parent = 0);

    static IconProvider* instance();

    void clearIconDatabase();

    void saveIcon(WebView* view);
    QImage iconForUrl(const QUrl &url);
    QImage iconForDomain(const QUrl &url);

    QIcon iconFromImage(const QImage &image);

    QIcon iconFromBase64(const QByteArray &data);
    QByteArray iconToBase64(const QIcon &icon);

    QIcon standardIcon(QStyle::StandardPixmap icon);
    QIcon fromTheme(const QString &icon);

    QIcon emptyWebIcon();
    QImage emptyWebImage();

    QPixmap bookmarkIcon();
    void setBookmarkIcon(const QPixmap &pixmap);

signals:

public slots:
    void saveIconsToDatabase();

private:
    QTimer* m_timer;

    struct Icon {
        QUrl url;
        QImage image;
    };

    static IconProvider* s_instance;

    QImage m_emptyWebImage;
    QPixmap m_bookmarkIcon;

    QVector<Icon> m_iconBuffer;
};

#endif // ICONPROVIDER_H
