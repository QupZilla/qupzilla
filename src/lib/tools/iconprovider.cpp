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
#include "mainapplication.h"
#include "sqldatabase.h"
#include "autosaver.h"
#include "webview.h"

#include <QTimer>
#include <QBuffer>

Q_GLOBAL_STATIC(IconProvider, qz_icon_provider)

IconProvider::IconProvider()
    : QWidget()
{
    m_autoSaver = new AutoSaver(this);
    connect(m_autoSaver, SIGNAL(save()), this, SLOT(saveIconsToDatabase()));
}

void IconProvider::saveIcon(WebView* view)
{
    // Don't save icons in private mode.
    if (mApp->isPrivate()) {
        return;
    }

    BufferedIcon item;
    item.first = view->url();
    item.second = view->icon().pixmap(16, 16).toImage();

    if (item.second == IconProvider::emptyWebImage()) {
        return;
    }

    if (m_iconBuffer.contains(item)) {
        return;
    }

    m_autoSaver->changeOcurred();
    m_iconBuffer.append(item);
}

QPixmap IconProvider::bookmarkIcon() const
{
    return m_bookmarkIcon;
}

void IconProvider::setBookmarkIcon(const QPixmap &pixmap)
{
    m_bookmarkIcon = pixmap;
}

QIcon IconProvider::standardIcon(QStyle::StandardPixmap icon)
{
    switch (icon) {
    case QStyle::SP_MessageBoxCritical:
        return QIcon::fromTheme("dialog-error", QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical));

    case QStyle::SP_MessageBoxInformation:
        return QIcon::fromTheme("dialog-information", QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation));

    case QStyle::SP_MessageBoxQuestion:
        return QIcon::fromTheme("dialog-question", QApplication::style()->standardIcon(QStyle::SP_MessageBoxQuestion));

    case QStyle::SP_MessageBoxWarning:
        return QIcon::fromTheme("dialog-warning", QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning));

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
        return QApplication::style()->standardIcon(icon);
    }
}

QIcon IconProvider::iconFromTheme(const QString &icon)
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
    return QPixmap::fromImage(instance()->m_emptyWebImage);
}

QImage IconProvider::emptyWebImage()
{
    if (instance()->m_emptyWebImage.isNull()) {
        instance()->m_emptyWebImage = iconFromTheme("text-plain").pixmap(16, 16).toImage();
    }

    return instance()->m_emptyWebImage;
}

QIcon IconProvider::iconForUrl(const QUrl &url)
{
    return instance()->iconFromImage(imageForUrl(url));
}

QImage IconProvider::imageForUrl(const QUrl &url)
{
    if (url.path().isEmpty()) {
        return IconProvider::emptyWebImage();
    }

    foreach (const BufferedIcon &ic, instance()->m_iconBuffer) {
        if (ic.first.toString().startsWith(url.toString())) {
            return ic.second;
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

QIcon IconProvider::iconForDomain(const QUrl &url)
{
    return instance()->iconFromImage(imageForDomain(url));
}

QImage IconProvider::imageForDomain(const QUrl &url)
{
    foreach (const BufferedIcon &ic, instance()->m_iconBuffer) {
        if (ic.first.host() == url.host()) {
            return ic.second;
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

IconProvider* IconProvider::instance()
{
    return qz_icon_provider();
}

void IconProvider::saveIconsToDatabase()
{
    foreach (const BufferedIcon &ic, m_iconBuffer) {
        QSqlQuery query;
        query.prepare("SELECT id FROM icons WHERE url = ?");
        query.bindValue(0, ic.first.toEncoded(QUrl::RemoveFragment));
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
        ic.second.save(&buffer, "PNG");
        query.bindValue(0, buffer.data());
        query.bindValue(1, ic.first.toEncoded(QUrl::RemoveFragment));

        SqlDatabase::instance()->execAsync(query);
    }

    m_iconBuffer.clear();
}

void IconProvider::clearIconsDatabase()
{
    QSqlQuery query;
    query.exec("DELETE FROM icons");
    query.exec("VACUUM");

    m_iconBuffer.clear();
}

QIcon IconProvider::iconFromImage(const QImage &image)
{
    if (m_emptyWebImage.isNull()) {
        m_emptyWebImage = iconFromTheme("text-plain").pixmap(16, 16).toImage();
    }

    return QIcon(QPixmap::fromImage(image));
}
