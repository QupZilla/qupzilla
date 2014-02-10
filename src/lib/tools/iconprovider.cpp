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
#include "iconprovider.h"
#include "webview.h"
#include "mainapplication.h"
#include "databasewriter.h"

#include <QTimer>
#include <QBuffer>

IconProvider* IconProvider::s_instance = 0;

IconProvider::IconProvider(QWidget* parent)
    : QWidget(parent)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(10 * 1000);
    m_timer->start();

    connect(m_timer, SIGNAL(timeout()), this, SLOT(saveIconsToDatabase()));
}

IconProvider* IconProvider::instance()
{
    if (!s_instance) {
        s_instance = new IconProvider;
    }

    return s_instance;
}

void IconProvider::saveIcon(WebView* view)
{
    if (mApp->isPrivateSession()) {
        // Don't save icons in private mode.
        return;
    }

    Icon item;
    item.image = view->icon().pixmap(16, 16).toImage();
    item.url = view->url();

    if (item.image == IconProvider::emptyWebImage()) {
        return;
    }

    foreach (const Icon &ic, m_iconBuffer) {
        if (ic.url == item.url && ic.image == item.image) {
            return;
        }
    }

    m_iconBuffer.append(item);
}

QImage IconProvider::iconForUrl(const QUrl &url)
{
    if (url.path().isEmpty()) {
        return IconProvider::emptyWebImage();
    }

    foreach (const Icon &ic, m_iconBuffer) {
        if (ic.url.toString().startsWith(url.toString())) {
            return ic.image;
        }
    }

    QSqlQuery query;
    query.prepare("SELECT icon FROM icons WHERE url LIKE ? LIMIT 1");
    query.addBindValue(QString("%1%").arg(QString::fromUtf8(url.toEncoded(QUrl::RemoveFragment))));
    query.exec();

    if (query.next()) {
        return QImage::fromData(query.value(0).toByteArray());
    }

    return IconProvider::emptyWebImage();
}

QImage IconProvider::iconForDomain(const QUrl &url)
{
    foreach (const Icon &ic, m_iconBuffer) {
        if (ic.url.host() == url.host()) {
            return ic.image;
        }
    }

    QSqlQuery query;
    query.prepare("SELECT icon FROM icons WHERE url LIKE ?");
    query.addBindValue(QString("%%1%").arg(url.host()));
    query.exec();

    if (query.next()) {
        return QImage::fromData(query.value(0).toByteArray());
    }

    return QImage();
}

void IconProvider::saveIconsToDatabase()
{
    foreach (const Icon &ic, m_iconBuffer) {
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
    switch (icon) {
    case QStyle::SP_MessageBoxCritical:
        return QIcon::fromTheme("dialog-error", mApp->style()->standardIcon(QStyle::SP_MessageBoxCritical));

    case QStyle::SP_MessageBoxInformation:
        return QIcon::fromTheme("dialog-information", mApp->style()->standardIcon(QStyle::SP_MessageBoxInformation));

    case QStyle::SP_MessageBoxQuestion:
        return QIcon::fromTheme("dialog-question", mApp->style()->standardIcon(QStyle::SP_MessageBoxQuestion));

    case QStyle::SP_MessageBoxWarning:
        return QIcon::fromTheme("dialog-warning", mApp->style()->standardIcon(QStyle::SP_MessageBoxWarning));

#ifndef QZ_WS_X11
    case QStyle::SP_DialogCloseButton:
        return QIcon(":/icons/theme/close.png");

    case QStyle::SP_BrowserStop:
        return QIcon(":/icons/theme/stop.png");

    case QStyle::SP_BrowserReload:
        return QIcon(":/icons/theme/reload.png");

    case QStyle::SP_FileDialogToParent:
        return QIcon(":/icons/theme/go-up.png");

    case QStyle::SP_ArrowForward:
        //RTL Support
        if (QApplication::layoutDirection() == Qt::RightToLeft) {
            return QIcon(":/icons/theme/back.png");
        }
        else {
            return QIcon(":/icons/theme/forward.png");
        }

    case QStyle::SP_ArrowBack:
        //RTL Support
        if (QApplication::layoutDirection() == Qt::RightToLeft) {
            return QIcon(":/icons/theme/forward.png");
        }
        else {
            return QIcon(":/icons/theme/back.png");
        }
#endif
    default:
        return mApp->style()->standardIcon(icon);
    }
}

QIcon IconProvider::fromTheme(const QString &icon)
{
    // TODO: This should actually look in :icons/theme for fallback icon, not hardcode every icon

    if (icon == QLatin1String("go-home")) {
        return QIcon::fromTheme("go-home", QIcon(":/icons/theme/home.png"));
    }
    else if (icon == QLatin1String("text-plain")) {
        return QIcon::fromTheme("text-plain", QIcon(":icons/locationbar/unknownpage.png"));
    }
    else if (icon == QLatin1String("bookmarks-organize")) {
        return QIcon::fromTheme("bookmarks-organize", QIcon(":icons/theme/user-bookmarks.png"));
    }
    else if (icon == QLatin1String("bookmark-new")) {
        return QIcon::fromTheme("bookmark-new", QIcon(":icons/theme/user-bookmarks.png"));
    }
    else if (icon == QLatin1String("list-remove")) {
        return QIcon::fromTheme("list-remove", QIcon(":icons/theme/list-remove.png"));
    }
    else if (icon == QLatin1String("go-next")) {
        return QIcon::fromTheme("go-next", QIcon(":icons/theme/go-next.png"));
    }
    else if (icon == QLatin1String("go-previous")) {
        return QIcon::fromTheme("go-previous", QIcon(":icons/theme/go-previous.png"));
    }
    else if (icon == QLatin1String("view-restore")) {
        return QIcon::fromTheme("view-restore", QIcon(":icons/theme/view-restore.png"));
    }
    else {
        return QIcon::fromTheme(icon);
    }
}

QIcon IconProvider::emptyWebIcon()
{
    return QPixmap::fromImage(m_emptyWebImage);
}

QImage IconProvider::emptyWebImage()
{
    if (m_emptyWebImage.isNull()) {
        m_emptyWebImage = fromTheme("text-plain").pixmap(16, 16).toImage();
    }

    return m_emptyWebImage;
}

QPixmap IconProvider::bookmarkIcon()
{
    return m_bookmarkIcon;
}

void IconProvider::setBookmarkIcon(const QPixmap &pixmap)
{
    m_bookmarkIcon = pixmap;
}

QIcon IconProvider::iconFromImage(const QImage &image)
{
    if (m_emptyWebImage.isNull()) {
        m_emptyWebImage = fromTheme("text-plain").pixmap(16, 16).toImage();
    }

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
    return IconProvider::emptyWebIcon();
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
