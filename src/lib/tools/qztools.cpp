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
#include "qztools.h"
#include "datapaths.h"
#include "settings.h"

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
#include <QProcess>
#include <QMessageBox>
#include <QWebFrame>

#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif
#ifdef QZ_WS_X11
#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#ifdef Q_OS_MAC
#include <CoreServices/CoreServices.h>
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
    return QString::fromUtf8(readAllFileByteContents(filename));
}

QByteArray QzTools::readAllFileByteContents(const QString &filename)
{
    QFile file(filename);

    if (!filename.isEmpty() && file.open(QFile::ReadOnly)) {
        const QByteArray a = file.readAll();
        file.close();
        return a;
    }

    return QByteArray();
}

void QzTools::centerWidgetOnScreen(QWidget* w)
{
    const QRect screen = QApplication::desktop()->screenGeometry();
    const QRect size = w->geometry();
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

QString QzTools::fromPunycode(const QString &str)
{
    if (!str.startsWith(QL1S("xn--")))
        return str;

    // QUrl::fromAce will only decode domains from idn whitelist
    const QString decoded = QUrl::fromAce(str.toUtf8() + QByteArray(".org"));
    return decoded.left(decoded.size() - 4);
}

QString QzTools::escapeSqlString(QString urlString)
{
    const static QString &escapeString = QL1S("!");
    urlString.replace(escapeString, escapeString + escapeString);
    urlString.replace(QL1S("_"), escapeString + QL1S("_"));
    urlString.replace(QL1S("%"), escapeString + QL1S("%"));

    return urlString;
}

QUrl QzTools::frameUrl(QWebFrame* frame)
{
    return frame->url().isEmpty() ? frame->baseUrl() : frame->url();
}

QString QzTools::ensureUniqueFilename(const QString &name, const QString &appendFormat)
{
    Q_ASSERT(appendFormat.contains(QL1S("%1")));

    QFileInfo info(name);

    if (!info.exists())
        return name;

    const QDir dir = info.absoluteDir();
    const QString fileName = info.fileName();

    int i = 1;

    while (info.exists()) {
        QString file = fileName;
        int index = file.lastIndexOf(QL1C('.'));
        const QString appendString = appendFormat.arg(i);
        if (index == -1)
            file.append(appendString);
        else
            file = file.left(index) + appendString + file.mid(index);
        info.setFile(dir, file);
        i++;
    }

    return info.absoluteFilePath();
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

QString QzTools::lastPathForFileDialog(const QString &dialogName, const QString &fallbackPath)
{
    Settings settings;
    settings.beginGroup("LastFileDialogsPaths");
    QString path = settings.value("FileDialogs/" + dialogName).toString();
    settings.endGroup();

    return path.isEmpty() ? fallbackPath : path;
}

void QzTools::saveLastPathForFileDialog(const QString &dialogName, const QString &path)
{
    if (path.isEmpty()) {
        return;
    }

    Settings settings;
    settings.beginGroup("LastFileDialogsPaths");
    settings.setValue(dialogName, path);
    settings.endGroup();
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
        return QString::number(_size > 1 ? _size : 1, 'f', 0) + " " + QObject::tr("KB");
    }

    _size /= 1024; // MB
    if (_size < 1000) {
        return QString::number(_size, 'f', 1) + " " + QObject::tr("MB");
    }

    _size /= 1024; // GB
    return QString::number(_size, 'f', 2) + " " + QObject::tr("GB");
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

QString QzTools::truncatedText(const QString &text, int size)
{
    if (text.length() > size) {
        return text.left(size) + QL1S("..");
    }
    return text;
}

// Thanks to http://www.qtcentre.org/threads/3205-Toplevel-widget-with-rounded-corners?p=17492#post17492
QRegion QzTools::roundedRect(const QRect &rect, int radius)
{
    QRegion region;

    // middle and borders
    region += rect.adjusted(radius, 0, -radius, 0);
    region += rect.adjusted(0, radius, 0, -radius);

    // top left
    QRect corner(rect.topLeft(), QSize(radius * 2, radius * 2));
    region += QRegion(corner, QRegion::Ellipse);

    // top right
    corner.moveTopRight(rect.topRight());
    region += QRegion(corner, QRegion::Ellipse);

    // bottom left
    corner.moveBottomLeft(rect.bottomLeft());
    region += QRegion(corner, QRegion::Ellipse);

    // bottom right
    corner.moveBottomRight(rect.bottomRight());
    region += QRegion(corner, QRegion::Ellipse);

    return region;
}

QIcon QzTools::iconFromFileName(const QString &fileName)
{
    static QHash<QString, QIcon> iconCache;

    QFileInfo tempInfo(fileName);
    if (iconCache.contains(tempInfo.suffix())) {
        return iconCache.value(tempInfo.suffix());
    }

    QFileIconProvider iconProvider;
    QTemporaryFile tempFile(DataPaths::path(DataPaths::Temp) + "/XXXXXX." + tempInfo.suffix());
    tempFile.open();
    tempInfo.setFile(tempFile.fileName());

    QIcon icon(iconProvider.icon(tempInfo));
    iconCache.insert(tempInfo.suffix(), icon);

    return icon;
}

QString QzTools::resolveFromPath(const QString &name)
{
    const QString path = qgetenv("PATH").trimmed();

    if (path.isEmpty()) {
        return QString();
    }

    QStringList dirs = path.split(QLatin1Char(':'), QString::SkipEmptyParts);

    foreach (const QString &dir, dirs) {
        QDir d(dir);
        if (d.exists(name)) {
            return d.absoluteFilePath(name);
        }
    }

    return QString();
}

// http://stackoverflow.com/questions/1031645/how-to-detect-utf-8-in-plain-c
bool QzTools::isUtf8(const char* string)
{
    if (!string) {
        return 0;
    }

    const unsigned char* bytes = (const unsigned char*)string;
    while (*bytes) {
        if ((// ASCII
                bytes[0] == 0x09 ||
                bytes[0] == 0x0A ||
                bytes[0] == 0x0D ||
                (0x20 <= bytes[0] && bytes[0] <= 0x7F)
            )
           ) {
            bytes += 1;
            continue;
        }

        if ((// non-overlong 2-byte
                (0xC2 <= bytes[0] && bytes[0] <= 0xDF) &&
                (0x80 <= bytes[1] && bytes[1] <= 0xBF)
            )
           ) {
            bytes += 2;
            continue;
        }

        if ((// excluding overlongs
                bytes[0] == 0xE0 &&
                (0xA0 <= bytes[1] && bytes[1] <= 0xBF) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF)
            ) ||
            (// straight 3-byte
                ((0xE1 <= bytes[0] && bytes[0] <= 0xEC) ||
                 bytes[0] == 0xEE ||
                 bytes[0] == 0xEF) &&
                (0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF)
            ) ||
            (// excluding surrogates
                bytes[0] == 0xED &&
                (0x80 <= bytes[1] && bytes[1] <= 0x9F) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF)
            )
           ) {
            bytes += 3;
            continue;
        }

        if ((// planes 1-3
                bytes[0] == 0xF0 &&
                (0x90 <= bytes[1] && bytes[1] <= 0xBF) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
                (0x80 <= bytes[3] && bytes[3] <= 0xBF)
            ) ||
            (// planes 4-15
                (0xF1 <= bytes[0] && bytes[0] <= 0xF3) &&
                (0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
                (0x80 <= bytes[3] && bytes[3] <= 0xBF)
            ) ||
            (// plane 16
                bytes[0] == 0xF4 &&
                (0x80 <= bytes[1] && bytes[1] <= 0x8F) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
                (0x80 <= bytes[3] && bytes[3] <= 0xBF)
            )
           ) {
            bytes += 4;
            continue;
        }

        return false;
    }

    return true;
}

bool QzTools::containsSpace(const QString &str)
{
    Q_FOREACH (const QChar &c, str) {
        if (c.isSpace())
            return true;
    }
    return false;
}

QString QzTools::getExistingDirectory(const QString &name, QWidget* parent, const QString &caption, const QString &dir, QFileDialog::Options options)
{
    Settings settings;
    settings.beginGroup("FileDialogPaths");

    QString lastDir = settings.value(name, dir).toString();

    QString path = QFileDialog::getExistingDirectory(parent, caption, lastDir, options);

    if (!path.isEmpty()) {
        settings.setValue(name, QFileInfo(path).absolutePath());
    }

    settings.endGroup();
    return path;
}

static QString getFilename(const QString &path)
{
    QFileInfo info(path);

    if (info.isFile()) {
        return info.fileName();
    }

    if (info.isDir()) {
        return QString();
    }

    if (info.dir().exists()) {
        return info.fileName();
    }

    return QString();
}

QString QzTools::getOpenFileName(const QString &name, QWidget* parent, const QString &caption, const QString &dir, const QString &filter, QString* selectedFilter, QFileDialog::Options options)
{
    Settings settings;
    settings.beginGroup("FileDialogPaths");

    QString lastDir = settings.value(name, QString()).toString();
    QString fileName = getFilename(dir);

    if (lastDir.isEmpty()) {
        lastDir = dir;
    }
    else {
        lastDir.append(QDir::separator() + fileName);
    }

    QString path = QFileDialog::getOpenFileName(parent, caption, lastDir, filter, selectedFilter, options);

    if (!path.isEmpty()) {
        settings.setValue(name, QFileInfo(path).absolutePath());
    }

    settings.endGroup();
    return path;
}

QStringList QzTools::getOpenFileNames(const QString &name, QWidget* parent, const QString &caption, const QString &dir, const QString &filter, QString* selectedFilter, QFileDialog::Options options)
{
    Settings settings;
    settings.beginGroup("FileDialogPaths");

    QString lastDir = settings.value(name, QString()).toString();
    QString fileName = getFilename(dir);

    if (lastDir.isEmpty()) {
        lastDir = dir;
    }
    else {
        lastDir.append(QDir::separator() + fileName);
    }

    QStringList paths = QFileDialog::getOpenFileNames(parent, caption, lastDir, filter, selectedFilter, options);

    if (!paths.isEmpty()) {
        settings.setValue(name, QFileInfo(paths.first()).absolutePath());
    }

    settings.endGroup();
    return paths;
}

QString QzTools::getSaveFileName(const QString &name, QWidget* parent, const QString &caption, const QString &dir, const QString &filter, QString* selectedFilter, QFileDialog::Options options)
{
    Settings settings;
    settings.beginGroup("FileDialogPaths");

    QString lastDir = settings.value(name, QString()).toString();
    QString fileName = getFilename(dir);

    if (lastDir.isEmpty()) {
        lastDir = dir;
    }
    else {
        lastDir.append(QDir::separator() + fileName);
    }

    QString path = QFileDialog::getSaveFileName(parent, caption, lastDir, filter, selectedFilter, options);

    if (!path.isEmpty()) {
        settings.setValue(name, QFileInfo(path).absolutePath());
    }

    settings.endGroup();
    return path;
}

// Matches domain (assumes both pattern and domain not starting with dot)
//  pattern = domain to be matched
//  domain = site domain
bool QzTools::matchDomain(const QString &pattern, const QString &domain)
{
    if (pattern == domain) {
        return true;
    }

    if (!domain.endsWith(pattern)) {
        return false;
    }

    int index = domain.indexOf(pattern);

    return index > 0 && domain[index - 1] == QLatin1Char('.');
}

static inline bool isQuote(const QChar &c)
{
    return (c == QLatin1Char('"') || c == QLatin1Char('\''));
}

// Function  splits command line into arguments
// eg. /usr/bin/foo -o test -b "bar bar" -s="sed sed"
//  => '/usr/bin/foo' '-o' 'test' '-b' 'bar bar' '-s=sed sed'
QStringList QzTools::splitCommandArguments(const QString &command)
{
    QString line = command.trimmed();

    if (line.isEmpty()) {
        return QStringList();
    }

    QChar SPACE(' ');
    QChar EQUAL('=');
    QChar BSLASH('\\');
    QChar QUOTE('"');
    QStringList r;

    int equalPos = -1; // Position of = in opt="value"
    int startPos = isQuote(line.at(0)) ? 1 : 0;
    bool inWord = !isQuote(line.at(0));
    bool inQuote = !inWord;

    if (inQuote) {
        QUOTE = line.at(0);
    }

    const int strlen = line.length();
    for (int i = 0; i < strlen; ++i) {
        const QChar c = line.at(i);

        if (inQuote && c == QUOTE && i > 0 && line.at(i - 1) != BSLASH) {
            QString str = line.mid(startPos, i - startPos);
            if (equalPos > -1) {
                str.remove(equalPos - startPos + 1, 1);
            }

            inQuote = false;
            if (!str.isEmpty()) {
                r.append(str);
            }
            continue;
        }
        else if (!inQuote && isQuote(c)) {
            inQuote = true;
            QUOTE = c;

            if (!inWord) {
                startPos = i + 1;
            }
            else if (i > 0 && line.at(i - 1) == EQUAL) {
                equalPos = i - 1;
            }
        }

        if (inQuote) {
            continue;
        }

        if (inWord && (c == SPACE || i == strlen - 1)) {
            int len = (i == strlen - 1) ? -1 : i - startPos;
            const QString str = line.mid(startPos, len);

            inWord = false;
            if (!str.isEmpty()) {
                r.append(str);
            }
        }
        else if (!inWord && c != SPACE) {
            inWord = true;
            startPos = i;
        }
    }

    // Unmatched quote
    if (inQuote) {
        return QStringList();
    }

    return r;
}

bool QzTools::startExternalProcess(const QString &executable, const QString &args)
{
    const QStringList arguments = splitCommandArguments(args);

    bool success = QProcess::startDetached(executable, arguments);

    if (!success) {
        QString info = "<ul><li><b>%1</b>%2</li><li><b>%3</b>%4</li></ul>";
        info = info.arg(QObject::tr("Executable: "), executable,
                        QObject::tr("Arguments: "), arguments.join(QLatin1String(" ")));

        QMessageBox::critical(0, QObject::tr("Cannot start external program"),
                              QObject::tr("Cannot start external program! %1").arg(info));
    }

    return success;
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

bool QzTools::isPlatformX11()
{
#if QZ_WS_X11 && QT_VERSION >= 0x050200
    return QX11Info::isPlatformX11();
#elif QZ_WS_X11
    return true;
#else
    return false;
#endif
}

void QzTools::setWmClass(const QString &name, const QWidget* widget)
{
#ifdef QZ_WS_X11
    if (isPlatformX11()) {
        QByteArray nameData = name.toUtf8();

        XClassHint classHint;
        classHint.res_name = const_cast<char*>(nameData.constData());
        classHint.res_class = const_cast<char*>("QupZilla");
        XSetClassHint(QX11Info::display(), widget->winId(), &classHint);
    }
#else
    Q_UNUSED(name)
    Q_UNUSED(widget)
#endif
}

QString QzTools::operatingSystem()
{
#ifdef Q_OS_MAC
    QString str = "Mac OS X";

    SInt32 majorVersion;
    SInt32 minorVersion;

    if (Gestalt(gestaltSystemVersionMajor, &majorVersion) == noErr && Gestalt(gestaltSystemVersionMinor, &minorVersion) == noErr) {
        str.append(QString(" %1.%2").arg(majorVersion).arg(minorVersion));
    }

    return str;
#endif
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
#ifdef Q_OS_UNIX
    return "Unix";
#endif
#ifdef Q_OS_HAIKU
    return "Haiku";
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
        OSVERSIONINFO osvi;
        ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        GetVersionEx(&osvi);

        if (osvi.dwMajorVersion == 6 &&  osvi.dwMinorVersion == 2) {
            str.append(" 8");
        }
        break;
    }

    return str;
#endif
}

