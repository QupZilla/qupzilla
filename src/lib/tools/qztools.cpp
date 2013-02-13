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
#include "qztools.h"
#include "mainapplication.h"

#include <QTextDocument>
#include <QDateTime>
#include <QByteArray>
#include <QPixmap>
#include <QPainter>
#include <QBuffer>
#include <QFile>
#include <QDir>
#include <QWidget>
#include <QApplication>
#include <QSslCertificate>
#include <QDesktopWidget>
#include <QUrl>
#include <QIcon>
#include <QFileIconProvider>
#include <QTemporaryFile>
#include <QHash>
#include <QSysInfo>

#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#include <qpa/qplatformnativeinterface.h>
#else
#include <QX11Info>
#endif

QByteArray QzTools::pixmapToByteArray(const QPixmap &pix)
{
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    if (pix.save(&buffer, "PNG")) {
        return buffer.buffer().toBase64();
    }

    return QByteArray();
}

QPixmap QzTools::pixmapFromByteArray(const QByteArray &data)
{
    QPixmap image;
    QByteArray bArray = QByteArray::fromBase64(data);
    image.loadFromData(bArray);

    return image;
}

QString QzTools::readAllFileContents(const QString &filename)
{
    QFile file(filename);
    if (file.open(QFile::ReadOnly)) {
        QString a = QString::fromUtf8(file.readAll());
        file.close();
        return a;
    }

    return QString();
}

void QzTools::centerWidgetOnScreen(QWidget* w)
{
    const QRect screen = QApplication::desktop()->screenGeometry();
    const QRect &size = w->geometry();
    w->move((screen.width() - size.width()) / 2, (screen.height() - size.height()) / 2);
}

// Very, very, very simplified QDialog::adjustPosition from qdialog.cpp
void QzTools::centerWidgetToParent(QWidget* w, QWidget* parent)
{
    if (!parent || !w) {
        return;
    }

    QPoint p;
    parent = parent->window();
    QPoint pp = parent->mapToGlobal(QPoint(0, 0));
    p = QPoint(pp.x() + parent->width() / 2, pp.y() + parent->height() / 2);
    p = QPoint(p.x() - w->width() / 2, p.y() - w->height() / 2 - 20);

    w->move(p);
}

bool QzTools::removeFile(const QString &fullFileName)
{
    QFile f(fullFileName);
    if (f.exists()) {
        return f.remove();
    }
    else {
        return false;
    }
}

void QzTools::removeDir(const QString &d)
{
    QDir dir(d);
    if (dir.exists()) {
        const QFileInfoList list = dir.entryInfoList();
        QFileInfo fi;
        for (int l = 0; l < list.size(); l++) {
            fi = list.at(l);
            if (fi.isDir() && fi.fileName() != QLatin1String(".") && fi.fileName() != QLatin1String("..")) {
                QzTools::removeDir(fi.absoluteFilePath());
            }
            else if (fi.isFile()) {
                QzTools::removeFile(fi.absoluteFilePath());
            }

        }
        dir.rmdir(d);
    }
}

/* Finds same part of @one in @other from the beginning */
QString QzTools::samePartOfStrings(const QString &one, const QString &other)
{
    int maxSize = qMin(one.size(), other.size());
    if (maxSize <= 0) {
        return QString();
    }

    int i = 0;
    while (one.at(i) == other.at(i)) {
        i++;
        if (i == maxSize) {
            break;
        }
    }
    return one.left(i);
}

QString QzTools::urlEncodeQueryString(const QUrl &url)
{
    QString returnString = url.toString(QUrl::RemoveQuery | QUrl::RemoveFragment);

    if (url.hasQuery()) {
#if QT_VERSION >= 0x050000
        returnString += QLatin1Char('?') + url.query(QUrl::FullyEncoded);
#else
        returnString += QLatin1Char('?') + url.encodedQuery();
#endif
    }

    if (url.hasFragment()) {
#if QT_VERSION >= 0x050000
        returnString += QLatin1Char('#') + url.fragment(QUrl::FullyEncoded);
#else
        returnString += QLatin1Char('#') + url.encodedFragment();
#endif
    }

    returnString.replace(QLatin1Char(' '), QLatin1String("%20"));

    return returnString;
}

QString QzTools::ensureUniqueFilename(const QString &name, const QString &appendFormat)
{
    if (!QFile::exists(name)) {
        return name;
    }

    QString tmpFileName = name;
    int i = 1;
    while (QFile::exists(tmpFileName)) {
        tmpFileName = name;
        int index = tmpFileName.lastIndexOf(QLatin1Char('.'));

        QString appendString = appendFormat.arg(i);
        if (index == -1) {
            tmpFileName.append(appendString);
        }
        else {
            tmpFileName = tmpFileName.left(index) + appendString + tmpFileName.mid(index);
        }
        i++;
    }
    return tmpFileName;
}

QString QzTools::getFileNameFromUrl(const QUrl &url)
{
    QString fileName = url.toString(QUrl::RemoveFragment | QUrl::RemoveQuery | QUrl::RemoveScheme | QUrl::RemovePort);

    if (fileName.endsWith(QLatin1Char('/'))) {
        fileName = fileName.mid(0, fileName.length() - 1);
    }

    if (fileName.indexOf(QLatin1Char('/')) != -1) {
        int pos = fileName.lastIndexOf(QLatin1Char('/'));
        fileName = fileName.mid(pos);
        fileName.remove(QLatin1Char('/'));
    }

    fileName = filterCharsFromFilename(fileName);

    if (fileName.isEmpty()) {
        fileName = filterCharsFromFilename(url.host());
    }

    return fileName;
}

QString QzTools::filterCharsFromFilename(const QString &name)
{
    QString value = name;

    value.replace(QLatin1Char('/'), QLatin1Char('-'));
    value.remove(QLatin1Char('\\'));
    value.remove(QLatin1Char(':'));
    value.remove(QLatin1Char('*'));
    value.remove(QLatin1Char('?'));
    value.remove(QLatin1Char('"'));
    value.remove(QLatin1Char('<'));
    value.remove(QLatin1Char('>'));
    value.remove(QLatin1Char('|'));

    return value;
}

QString QzTools::alignTextToWidth(const QString &string, const QString &text, const QFontMetrics &metrics, int width)
{
    int pos = 0;
    QString returnString;

    while (pos <= string.size()) {
        QString part = string.mid(pos);
        QString elidedLine = metrics.elidedText(part, Qt::ElideRight, width);

        if (elidedLine.isEmpty()) {
            break;
        }

        if (elidedLine.size() != part.size()) {
            elidedLine = elidedLine.left(elidedLine.size() - 3);
        }

        if (!returnString.isEmpty()) {
            returnString += text;
        }

        returnString += elidedLine;
        pos += elidedLine.size();
    }

    return returnString;
}

QString QzTools::fileSizeToString(qint64 size)
{
    if (size < 0) {
        return QObject::tr("Unknown size");
    }

    double _size = size / 1024.0; // KB
    if (_size < 1000) {
        return QString::number(_size > 1 ? _size : 1, 'f', 0) + " KB";
    }

    _size /= 1024; // MB
    if (_size < 1000) {
        return QString::number(_size, 'f', 1) + " MB";
    }

    _size /= 1024; // GB
    return QString::number(_size, 'f', 2) + " GB";
}


QPixmap QzTools::createPixmapForSite(const QIcon &icon, const QString &title, const QString &url)
{
    const QFontMetrics fontMetrics = QApplication::fontMetrics();
    const int padding = 4;
    const int maxWidth = fontMetrics.width(title.length() > url.length() ? title : url) + 3 * padding + 16;

    const int width = qMin(maxWidth, 150);
    const int height = fontMetrics.height() * 2 + fontMetrics.leading() + 2 * padding;

    QPixmap pixmap(width, height);
    QPainter painter(&pixmap);

    // Draw background
    QPen pen(Qt::black);
    pen.setWidth(1);
    painter.setPen(pen);

    painter.fillRect(QRect(0, 0, width, height), Qt::white);
    painter.drawRect(0, 0, width - 1, height - 1);

    // Draw icon
    QRect iconRect(0, 0, 16 + 2 * padding, height);
    icon.paint(&painter, iconRect);

    // Draw title
    QRect titleRect(iconRect.width(), padding, width - padding - iconRect.width(), fontMetrics.height());
    painter.drawText(titleRect, fontMetrics.elidedText(title, Qt::ElideRight, titleRect.width()));

    // Draw url
    QRect urlRect(titleRect.x(), titleRect.bottom() + fontMetrics.leading(), titleRect.width(), titleRect.height());
    painter.setPen(QApplication::palette().color(QPalette::Link));
    painter.drawText(urlRect, fontMetrics.elidedText(url, Qt::ElideRight, urlRect.width()));

    return pixmap;
}

QString QzTools::applyDirectionToPage(QString &pageContents)
{
    QString direction = QLatin1String("ltr");
    QString right_str = QLatin1String("right");
    QString left_str = QLatin1String("left");

    if (QApplication::isRightToLeft()) {
        direction = QLatin1String("rtl");
        right_str = QLatin1String("left");
        left_str = QLatin1String("right");
    }

    pageContents.replace(QLatin1String("%DIRECTION%"), direction);
    pageContents.replace(QLatin1String("%RIGHT_STR%"), right_str);
    pageContents.replace(QLatin1String("%LEFT_STR%"), left_str);

    return pageContents;
}

QIcon QzTools::iconFromFileName(const QString &fileName)
{
    static QHash<QString, QIcon> iconCache;

    QFileInfo tempInfo(fileName);
    if (iconCache.contains(tempInfo.suffix())) {
        return iconCache.value(tempInfo.suffix());
    }

    QFileIconProvider iconProvider;
    QTemporaryFile tempFile(mApp->tempPath() + "/XXXXXX." + tempInfo.suffix());
    tempFile.open();
    tempInfo.setFile(tempFile.fileName());

    QIcon icon(iconProvider.icon(tempInfo));
    iconCache.insert(tempInfo.suffix(), icon);

    return icon;
}

QString QT_QUPZILLA_EXPORT QzTools::resolveFromPath(const QString &name)
{
    const QString &path = qgetenv("PATH").trimmed();

    if (path.isEmpty()) {
        return QString();
    }

    QStringList dirs = path.split(QLatin1Char(':'), QString::SkipEmptyParts);

    foreach(const QString & dir, dirs) {
        QDir d(dir);
        if (d.exists(name)) {
            return d.absoluteFilePath(name);
        }
    }

    return QString();
}

// Qt5 migration help functions
bool QzTools::isCertificateValid(const QSslCertificate &cert)
{
#if QT_VERSION >= 0x050000
    const QDateTime currentTime = QDateTime::currentDateTime();
    return currentTime >= cert.effectiveDate() &&
           currentTime <= cert.expiryDate() &&
           !cert.isBlacklisted();
#else
    return cert.isValid();
#endif
}

QString QzTools::escape(const QString &string)
{
#if QT_VERSION >= 0x050000
    return string.toHtmlEscaped();
#else
    return Qt::escape(string);
#endif
}

#ifdef QZ_WS_X11
void* QzTools::X11Display(const QWidget* widget)
{
    Q_UNUSED(widget)

#if QT_VERSION >= 0x050000
    return qApp->platformNativeInterface()->nativeResourceForWindow("display", widget->windowHandle());
#else
    return QX11Info::display();
#endif
}
#endif

QString QzTools::buildSystem()
{
#ifdef Q_OS_LINUX
    return "Linux";
#endif
#ifdef Q_OS_BSD4
    return "BSD 4.4";
#endif
#ifdef Q_OS_BSDI
    return "BSD/OS";
#endif
#ifdef Q_OS_FREEBSD
    return "FreeBSD";
#endif
#ifdef Q_OS_HPUX
    return "HP-UX";
#endif
#ifdef Q_OS_HURD
    return "GNU Hurd";
#endif
#ifdef Q_OS_LYNX
    return "LynxOS";
#endif
#ifdef Q_OS_MAC
    return "MAC OS";
#endif
#ifdef Q_OS_NETBSD
    return "NetBSD";
#endif
#ifdef Q_OS_OS2
    return "OS/2";
#endif
#ifdef Q_OS_OPENBSD
    return "OpenBSD";
#endif
#ifdef Q_OS_OSF
    return "HP Tru64 UNIX";
#endif
#ifdef Q_OS_SOLARIS
    return "Sun Solaris";
#endif
#ifdef Q_OS_UNIXWARE
    return "UnixWare 7 / Open UNIX 8";
#endif
#ifdef Q_OS_WIN32
    QString str = "Windows";

    switch (QSysInfo::windowsVersion()) {
    case QSysInfo::WV_NT:
        str.append(" NT");
        break;

    case QSysInfo::WV_2000:
        str.append(" 2000");
        break;

    case QSysInfo::WV_XP:
        str.append(" XP");
        break;
    case QSysInfo::WV_2003:
        str.append(" XP Pro x64");
        break;

    case QSysInfo::WV_VISTA:
        str.append(" Vista");
        break;

    case QSysInfo::WV_WINDOWS7:
        str.append(" 7");
        break;

    default:
        break;
    }

    return str;
#endif
#ifdef Q_OS_UNIX
    return "Unix";
#endif
#ifdef Q_OS_HAIKU
    return "Haiku";
#endif
}
