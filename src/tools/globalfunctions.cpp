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
    return fileName;
}

QString qz_filterCharsFromFilename(const QString &name)
{
    QString value = name;
    value.remove("\\");
    value.remove("/");
    value.remove(":");
    value.remove("*");
    value.remove("?");
    value.remove("\"");
    value.remove("<");
    value.remove(">");
    value.remove("|");

    return value;
}

QString qz_buildSystem()
{
#ifdef Q_OS_LINUX
    return "Linux";
#endif
#ifdef Q_OS_UNIX
    return "Unix";
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
}
