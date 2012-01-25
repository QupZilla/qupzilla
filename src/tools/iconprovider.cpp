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
#include "iconprovider.h"
#include "webview.h"
#include "mainapplication.h"
#include "databasewriter.h"

IconProvider::IconProvider(QObject* parent)
    : QObject(parent)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(10 * 1000);
    m_timer->start();

    connect(m_timer, SIGNAL(timeout()), this, SLOT(saveIconsToDatabase()));
}

void IconProvider::saveIcon(WebView* view)
{
    if (mApp->webSettings()->testAttribute(QWebSettings::PrivateBrowsingEnabled)) {
        // Don't save icons in private mode.
        return;
    }

    Icon item;
    item.image = view->icon().pixmap(16, 16).toImage();
    item.url = view->url();

    if (item.image == QWebSettings::webGraphic(QWebSettings::DefaultFrameIconGraphic).toImage()) {
        return;
    }

    foreach(const Icon & ic, m_iconBuffer) {
        if (ic.url == item.url && ic.image == item.image) {
            return;
        }
    }

    m_iconBuffer.append(item);
}

QImage IconProvider::iconForUrl(const QUrl &url)
{
    foreach(const Icon & ic, m_iconBuffer) {
        if (ic.url == url) {
            return ic.image;
        }
    }

    QSqlQuery query;
    query.prepare("SELECT icon FROM icons WHERE url=?");
    query.bindValue(0, url.toEncoded(QUrl::RemoveFragment));
    query.exec();
    if (query.next()) {
        return QImage::fromData(query.value(0).toByteArray());
    }

    return QWebSettings::webGraphic(QWebSettings::DefaultFrameIconGraphic).toImage();
}

QImage IconProvider::iconForDomain(const QUrl &url)
{
    foreach(const Icon & ic, m_iconBuffer) {
        if (ic.url.host() == url.host()) {
            return ic.image;
        }
    }

    QSqlQuery query;
    query.exec("SELECT icon FROM icons WHERE url LIKE '%" + url.host() + "%'");
    if (query.next()) {
        return QImage::fromData(query.value(0).toByteArray());
    }

    return QImage();
}

void IconProvider::saveIconsToDatabase()
{
    foreach(const Icon & ic, m_iconBuffer) {
        QSqlQuery query;
        query.prepare("SELECT id FROM icons WHERE url = ?");
        query.bindValue(0, ic.url.toEncoded(QUrl::RemoveFragment));
        query.exec();

        if (query.next()) {
            query.prepare("UPDATE icons SET icon = ? WHERE url = ?");
        }
        else {
            query.prepare("INSERT INTO icons (icon, url) VALUES (?,?)");
        }

        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        ic.image.save(&buffer, "PNG");
        query.bindValue(0, buffer.data());
        query.bindValue(1, ic.url.toEncoded(QUrl::RemoveFragment));
        mApp->dbWriter()->executeQuery(query);
    }

    m_iconBuffer.clear();
}

void IconProvider::clearIconDatabase()
{
    QSqlQuery query;
    query.exec("DELETE FROM icons");
    query.exec("VACUUM");

    m_iconBuffer.clear();
}

QIcon IconProvider::standardIcon(QStyle::StandardPixmap icon)
{
#ifdef Q_WS_X11
    return mApp->style()->standardIcon(icon);
#else
    switch (icon) {
    case QStyle::SP_DialogCloseButton:
        return QIcon(":/icons/faenza/close.png");

    case QStyle::SP_BrowserStop:
        return QIcon(":/icons/faenza/stop.png");

    case QStyle::SP_BrowserReload:
        return QIcon(":/icons/faenza/reload.png");

    case QStyle::SP_ArrowForward:
        return QIcon(":/icons/faenza/forward.png");

    case QStyle::SP_ArrowBack:
        return QIcon(":/icons/faenza/back.png");

    default:
        return QIcon();
        break;
    }
#endif
}

QPixmap IconProvider::standardPixmap(QStyle::StandardPixmap icon)
{
#ifdef Q_WS_X11
    return mApp->style()->standardPixmap(icon);
#else
    switch (icon) {
    case QStyle::SP_DialogCloseButton:
        return QPixmap(":/icons/faenza/close.png");

    case QStyle::SP_BrowserStop:
        return QPixmap(":/icons/faenza/stop.png");

    case QStyle::SP_BrowserReload:
        return QPixmap(":/icons/faenza/reload.png");

    case QStyle::SP_ArrowForward:
        return QPixmap(":/icons/faenza/forward.png");

    case QStyle::SP_ArrowBack:
        return QPixmap(":/icons/faenza/back.png");

    default:
        return QPixmap();
        break;
    }
#endif
}

QIcon IconProvider::fromTheme(const QString &icon)
{
    if (icon == "go-home") {
        return QIcon::fromTheme("go-home", QIcon(":/icons/faenza/home.png"));
    }
    else if (icon == "text-plain") {
        return QIcon::fromTheme("text-plain", QIcon(":icons/locationbar/unknownpage.png"));
    }
    else if (icon == "user-bookmarks") {
        return QIcon::fromTheme("user-bookmarks", QIcon(":icons/faenza/user-bookmarks.png"));
    }
    else if (icon == "list-remove") {
        return QIcon::fromTheme("list-remove", QIcon(":icons/faenza/list-remove.png"));
    }
    else if (icon == "go-next") {
        return QIcon::fromTheme("go-next", QIcon(":icons/faenza/go-next.png"));
    }
    else if (icon == "go-previous") {
        return QIcon::fromTheme("go-previous", QIcon(":icons/faenza/go-previous.png"));
    }
    else {
        return QIcon::fromTheme(icon);
    }
}

QIcon IconProvider::iconFromImage(const QImage &image)
{
    return QIcon(QPixmap::fromImage(image));
}

QIcon IconProvider::iconFromBase64(const QByteArray &data)
{
    QIcon image;
    QByteArray bArray = QByteArray::fromBase64(data);
    QBuffer buffer(&bArray);
    buffer.open(QIODevice::ReadOnly);
    QDataStream in(&buffer);
    in >> image;
    buffer.close();

    if (!image.isNull()) {
        return image;
    }
    return QWebSettings::webGraphic(QWebSettings::DefaultFrameIconGraphic);
}

QByteArray IconProvider::iconToBase64(const QIcon &icon)
{
    QByteArray bArray;
    QBuffer buffer(&bArray);
    buffer.open(QIODevice::WriteOnly);
    QDataStream out(&buffer);
    out << icon;
    buffer.close();
    return bArray.toBase64();
}

IconProvider::~IconProvider()
{
}
