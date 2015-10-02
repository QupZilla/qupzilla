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
#include "networkmanager.h"
#include "sqldatabase.h"
#include "autosaver.h"
#include "webview.h"
#include "qztools.h"

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

    if (view->url().scheme() == QL1S("qupzilla"))
        return;

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
        return QIcon::fromTheme(QSL("dialog-error"), QApplication::style()->standardIcon(icon));

    case QStyle::SP_MessageBoxInformation:
        return QIcon::fromTheme(QSL("dialog-information"), QApplication::style()->standardIcon(icon));

    case QStyle::SP_MessageBoxQuestion:
        return QIcon::fromTheme(QSL("dialog-question"), QApplication::style()->standardIcon(icon));

    case QStyle::SP_MessageBoxWarning:
        return QIcon::fromTheme(QSL("dialog-warning"), QApplication::style()->standardIcon(icon));

    case QStyle::SP_DialogCloseButton:
        return QIcon::fromTheme(QSL("dialog-close"), QApplication::style()->standardIcon(icon));

    case QStyle::SP_BrowserStop:
        return QIcon::fromTheme(QSL("process-stop"), QApplication::style()->standardIcon(icon));

    case QStyle::SP_BrowserReload:
        return QIcon::fromTheme(QSL("view-refresh"), QApplication::style()->standardIcon(icon));

    case QStyle::SP_FileDialogToParent:
        return QIcon::fromTheme(QSL("go-up"), QApplication::style()->standardIcon(icon));

    case QStyle::SP_ArrowUp:
        return QIcon::fromTheme(QSL("go-up"), QApplication::style()->standardIcon(icon));

    case QStyle::SP_ArrowDown:
        return QIcon::fromTheme(QSL("go-down"), QApplication::style()->standardIcon(icon));

    case QStyle::SP_ArrowForward:
        if (QApplication::layoutDirection() == Qt::RightToLeft) {
            return QIcon::fromTheme(QSL("go-previous"), QApplication::style()->standardIcon(icon));
        }
        return QIcon::fromTheme(QSL("go-next"), QApplication::style()->standardIcon(icon));

    case QStyle::SP_ArrowBack:
        if (QApplication::layoutDirection() == Qt::RightToLeft) {
            return QIcon::fromTheme(QSL("go-next"), QApplication::style()->standardIcon(icon));
        }
        return QIcon::fromTheme(QSL("go-previous"), QApplication::style()->standardIcon(icon));

    default:
        return QApplication::style()->standardIcon(icon);
    }
}

QIcon IconProvider::newTabIcon()
{
    return QIcon::fromTheme(QSL("tab-new"), QIcon(QSL(":/icons/menu/tab-new.png")));
}

QIcon IconProvider::newWindowIcon()
{
    return QIcon::fromTheme(QSL("window-new"), QIcon(QSL(":/icons/menu/window-new.png")));
}

QIcon IconProvider::privateBrowsingIcon()
{
    return QIcon(QSL(":/icons/menu/privatebrowsing.png"));
}

QIcon IconProvider::settingsIcon()
{
    return QIcon(QSL(":/icons/menu/settings.png"));
}

QIcon IconProvider::emptyWebIcon()
{
    return QPixmap::fromImage(instance()->emptyWebImage());
}

QImage IconProvider::emptyWebImage()
{
    if (instance()->m_emptyWebImage.isNull()) {
        instance()->m_emptyWebImage = QPixmap(":icons/other/empty-page.png").toImage();
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
    query.prepare(QSL("SELECT icon FROM icons WHERE url LIKE ? ESCAPE ? LIMIT 1"));

    query.addBindValue(QString("%1%").arg(QzTools::escapeSqlString(QString::fromUtf8(url.toEncoded(QUrl::RemoveFragment)))));
    query.addBindValue(QL1S("!"));
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
    query.prepare(QSL("SELECT icon FROM icons WHERE url LIKE ? ESCAPE ? LIMIT 1"));

    query.addBindValue(QString("%%1%").arg(QzTools::escapeSqlString(url.host())));
    query.addBindValue(QL1S("!"));
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
    return QIcon(QPixmap::fromImage(image));
}
