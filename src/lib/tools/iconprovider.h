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
#ifndef ICONPROVIDER_H
#define ICONPROVIDER_H

#include <QWidget>
#include <QStyle>
#include <QImage>
#include <QUrl>

#include "qzcommon.h"

class QIcon;

class WebView;
class AutoSaver;

// Needs to be QWidget subclass, otherwise qproperty- setting won't work
class QUPZILLA_EXPORT IconProvider : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QPixmap bookmarkIcon READ bookmarkIcon WRITE setBookmarkIcon)

public:
    explicit IconProvider();

    void saveIcon(WebView* view);

    QPixmap bookmarkIcon() const;
    void setBookmarkIcon(const QPixmap &pixmap);

    // QStyle equivalent
    static QIcon standardIcon(QStyle::StandardPixmap icon);

    static QIcon newTabIcon();
    static QIcon newWindowIcon();
    static QIcon privateBrowsingIcon();
    static QIcon settingsIcon();

    // Icon for empty page
    static QIcon emptyWebIcon();
    static QImage emptyWebImage();

    // Icon for url (only available for urls in history)
    static QIcon iconForUrl(const QUrl &url);
    static QImage imageForUrl(const QUrl &url);

    // Icon for domain (only available for urls in history)
    static QIcon iconForDomain(const QUrl &url);
    static QImage imageForDomain(const QUrl &url);

    static IconProvider* instance();

public slots:
    void saveIconsToDatabase();
    void clearIconsDatabase();

private:
    typedef QPair<QUrl, QImage> BufferedIcon;

    QIcon iconFromImage(const QImage &image);

    QImage m_emptyWebImage;
    QPixmap m_bookmarkIcon;
    QVector<BufferedIcon> m_iconBuffer;

    AutoSaver* m_autoSaver;
};

#endif // ICONPROVIDER_H
