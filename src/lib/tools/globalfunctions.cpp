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
#include "globalfunctions.h"

#include <QByteArray>
#include <QPixmap>
#include <QPainter>
#include <QBuffer>
#include <QFile>
#include <QDir>
#include <QWidget>
#include <QApplication>
#include <QDesktopWidget>
#include <QUrl>
#include <QIcon>

QByteArray qz_pixmapToByteArray(const QPixmap &pix)
{
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    if (pix.save(&buffer, "PNG")) {
        return buffer.buffer().toBase64();
    }

    return QByteArray();
}

QPixmap qz_pixmapFromByteArray(const QByteArray &data)
{
    QPixmap image;
    QByteArray bArray = QByteArray::fromBase64(data);
    image.loadFromData(bArray);

    return image;
}

QByteArray qz_readAllFileContents(const QString &filename)
{
    QFile file(filename);
    file.open(QFile::ReadOnly);
    QByteArray a = file.readAll();
    file.close();

    return a;
}

void qz_centerWidgetOnScreen(QWidget* w)
{
    const QRect screen = QApplication::desktop()->screenGeometry();
    const QRect &size = w->geometry();
    w->move((screen.width() - size.width()) / 2, (screen.height() - size.height()) / 2);
}

// Very, very, very simplified QDialog::adjustPosition from qdialog.cpp
void qz_centerWidgetToParent(QWidget* w, QWidget* parent)
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

bool qz_removeFile(const QString &fullFileName)
{
    QFile f(fullFileName);
    if (f.exists()) {
        return f.remove();
    }
    else {
        return false;
    }
}

void qz_removeDir(const QString &d)
{
    QDir dir(d);
    if (dir.exists()) {
        const QFileInfoList list = dir.entryInfoList();
        QFileInfo fi;
        for (int l = 0; l < list.size(); l++) {
            fi = list.at(l);
            if (fi.isDir() && fi.fileName() != "." && fi.fileName() != "..") {
                qz_removeDir(fi.absoluteFilePath());
            }
            else if (fi.isFile()) {
                qz_removeFile(fi.absoluteFilePath());
            }

        }
        dir.rmdir(d);
    }
}


QString qz_samePartOfStrings(const QString &one, const QString &other)
{
    int i = 0;
    int maxSize = qMin(one.size(), other.size());
    while (one.at(i) == other.at(i)) {
        i++;
        if (i == maxSize) {
            break;
        }
    }
    return one.left(i);
}

QUrl qz_makeRelativeUrl(const QUrl &baseUrl, const QUrl &rUrl)
{
    QString baseUrlPath = baseUrl.path();
    QString rUrlPath = rUrl.path();

    QString samePart = qz_samePartOfStrings(baseUrlPath, rUrlPath);

    QUrl returnUrl;
    if (samePart.isEmpty()) {
        returnUrl = rUrl;
    }
    else if (samePart == "/") {
        returnUrl = QUrl(rUrl.path());
    }
    else {
        samePart = samePart.left(samePart.lastIndexOf("/") + 1);
        int slashCount = samePart.count("/") + 1;
        if (samePart.startsWith("/")) {
            slashCount--;
        }
        if (samePart.endsWith("/")) {
            slashCount--;
        }

        rUrlPath.remove(samePart);
        rUrlPath.prepend(QString("..""/").repeated(slashCount));
        returnUrl = QUrl(rUrlPath);
    }

    return returnUrl;
}

QString qz_urlEncodeQueryString(const QUrl &url)
{
    QString returnString = url.toString(QUrl::RemoveQuery | QUrl::RemoveFragment);

    if (url.hasQuery()) {
        returnString += "?" + url.encodedQuery();
    }

    if (url.hasFragment()) {
        returnString += "#" + url.encodedFragment();
    }

    returnString.replace(" ", "%20");

    return returnString;
}

QString qz_ensureUniqueFilename(const QString &pathToFile)
{
    if (!QFile::exists(pathToFile)) {
        return pathToFile;
    }

    QString tmpFileName = pathToFile;
    int i = 1;
    while (QFile::exists(tmpFileName)) {
        tmpFileName = pathToFile;
        int index = tmpFileName.lastIndexOf(".");

        if (index == -1) {
            tmpFileName.append("(" + QString::number(i) + ")");
        }
        else {
            tmpFileName = tmpFileName.mid(0, index) + "(" + QString::number(i) + ")" + tmpFileName.mid(index);
        }
        i++;
    }
    return tmpFileName;
}

QString qz_getFileNameFromUrl(const QUrl &url)
{
    QString fileName = url.toString(QUrl::RemoveFragment | QUrl::RemoveQuery | QUrl::RemoveScheme | QUrl::RemovePort);
    if (fileName.indexOf("/") != -1) {
        int pos = fileName.lastIndexOf("/");
        fileName = fileName.mid(pos);
        fileName.remove("/");
    }

    fileName = qz_filterCharsFromFilename(fileName);

    if (fileName.isEmpty()) {
        fileName = qz_filterCharsFromFilename(url.host().replace(".", "-"));
    }

    return fileName;
}

QString qz_filterCharsFromFilename(const QString &name)
{
    QString value = name;
    value.replace("/", "-");
    value.remove("\\");
    value.remove(":");
    value.remove("*");
    value.remove("?");
    value.remove("\"");
    value.remove("<");
    value.remove(">");
    value.remove("|");

    return value;
}

QString qz_alignTextToWidth(const QString &string, const QString &text, const QFontMetrics &metrics, int width)
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
            elidedLine = elidedLine.mid(0, elidedLine.size() - 3);
        }

        if (!returnString.isEmpty()) {
            returnString += text;
        }

        returnString += elidedLine;
        pos += elidedLine.size();
    }

    return returnString;
}

QPixmap qz_createPixmapForSite(const QIcon &icon, const QString &title, const QString &url)
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

QString qz_buildSystem()
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
    return "Windows";
#endif
#ifdef Q_OS_UNIX
    return "Unix";
#endif
#ifdef Q_OS_HAIKU
    return "Haiku";
#endif
}
