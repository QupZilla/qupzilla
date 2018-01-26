/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018 David Rosca <nowrep@gmail.com>
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
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrentRun>

Q_GLOBAL_STATIC(IconProvider, qz_icon_provider)

static QByteArray encodeUrl(const QUrl &url)
{
    return url.toEncoded(QUrl::RemoveFragment | QUrl::StripTrailingSlash);
}

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

    const QIcon icon = view->icon(true);
    if (icon.isNull()) {
        return;
    }

    const QStringList ignoredSchemes = {
        QStringLiteral("qupzilla"),
        QStringLiteral("ftp"),
        QStringLiteral("file"),
        QStringLiteral("view-source"),
        QStringLiteral("data"),
        QStringLiteral("about")
    };

    if (ignoredSchemes.contains(view->url().scheme())) {
        return;
    }

    for (int i = 0; i < m_iconBuffer.size(); ++i) {
        if (m_iconBuffer[i].first == view->url()) {
            m_iconBuffer.removeAt(i);
            break;
        }
    }

    BufferedIcon item;
    item.first = view->url();
    item.second = icon.pixmap(16).toImage();

    m_autoSaver->changeOccurred();
    m_iconBuffer.append(item);
}

QIcon IconProvider::bookmarkIcon() const
{
    return QIcon::fromTheme(QSL("bookmarks"), m_bookmarkIcon);
}

void IconProvider::setBookmarkIcon(const QIcon &icon)
{
    m_bookmarkIcon = icon;
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
    return QIcon::fromTheme(QSL("tab-new"), QIcon(QSL(":/icons/menu/tab-new.svg")));
}

QIcon IconProvider::newWindowIcon()
{
    return QIcon::fromTheme(QSL("window-new"), QIcon(QSL(":/icons/menu/window-new.svg")));
}

QIcon IconProvider::privateBrowsingIcon()
{
    return QIcon(QSL(":/icons/menu/privatebrowsing.png"));
}

QIcon IconProvider::settingsIcon()
{
    return QIcon::fromTheme(QSL("configure"), QIcon(QSL(":/icons/menu/settings.svg")));
}

QIcon IconProvider::emptyWebIcon()
{
    return QPixmap::fromImage(instance()->emptyWebImage());
}

QImage IconProvider::emptyWebImage()
{
    if (instance()->m_emptyWebImage.isNull()) {
        instance()->m_emptyWebImage = QIcon(QSL(":icons/other/webpage.svg")).pixmap(16).toImage();
    }

    return instance()->m_emptyWebImage;
}

QIcon IconProvider::iconForUrl(const QUrl &url, bool allowNull)
{
    return instance()->iconFromImage(imageForUrl(url, allowNull));
}

QImage IconProvider::imageForUrl(const QUrl &url, bool allowNull)
{
    if (url.path().isEmpty()) {
        return allowNull ? QImage() : IconProvider::emptyWebImage();
    }

    const QByteArray encodedUrl = encodeUrl(url);

    foreach (const BufferedIcon &ic, instance()->m_iconBuffer) {
        if (encodeUrl(ic.first) == encodedUrl) {
            return ic.second;
        }
    }

    QSqlQuery query(SqlDatabase::instance()->database());
    query.prepare(QSL("SELECT icon FROM icons WHERE url GLOB ? LIMIT 1"));
    query.addBindValue(QString("%1*").arg(QzTools::escapeSqlGlobString(QString::fromUtf8(encodedUrl))));
    query.exec();

    if (query.next()) {
        return QImage::fromData(query.value(0).toByteArray());
    }

    return allowNull ? QImage() : IconProvider::emptyWebImage();
}

QIcon IconProvider::iconForDomain(const QUrl &url, bool allowNull)
{
    return instance()->iconFromImage(imageForDomain(url, allowNull));
}

QImage IconProvider::imageForDomain(const QUrl &url, bool allowNull)
{
    if (url.host().isEmpty()) {
        return allowNull ? QImage() : IconProvider::emptyWebImage();
    }

    foreach (const BufferedIcon &ic, instance()->m_iconBuffer) {
        if (ic.first.host() == url.host()) {
            return ic.second;
        }
    }

    QSqlQuery query(SqlDatabase::instance()->database());
    query.prepare(QSL("SELECT icon FROM icons WHERE url GLOB ? LIMIT 1"));
    query.addBindValue(QString("*%1*").arg(QzTools::escapeSqlGlobString(url.host())));
    query.exec();

    if (query.next()) {
        return QImage::fromData(query.value(0).toByteArray());
    }

    return allowNull ? QImage() : IconProvider::emptyWebImage();
}

IconProvider* IconProvider::instance()
{
    return qz_icon_provider();
}

void IconProvider::saveIconsToDatabase()
{
    foreach (const BufferedIcon &ic, m_iconBuffer) {
        QSqlQuery query(SqlDatabase::instance()->database());
        query.prepare("SELECT id FROM icons WHERE url = ?");
        query.bindValue(0, encodeUrl(ic.first));
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
        query.bindValue(1, QString::fromUtf8(encodeUrl(ic.first)));
        query.exec();
    }

    m_iconBuffer.clear();
}

void IconProvider::clearOldIconsInDatabase()
{
    // Delete icons for entries older than 6 months
    const QDateTime date = QDateTime::currentDateTime().addMonths(-6);

    QSqlQuery query(SqlDatabase::instance()->database());
    query.prepare(QSL("DELETE FROM icons WHERE url IN (SELECT url FROM history WHERE date < ?)"));
    query.addBindValue(date.toMSecsSinceEpoch());
    query.exec();

    query.clear();
    query.exec(QSL("VACUUM"));
}

QIcon IconProvider::iconFromImage(const QImage &image)
{
    return QIcon(QPixmap::fromImage(image));
}
