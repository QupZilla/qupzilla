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
#ifndef QZTOOLS_H
#define QZTOOLS_H

#include <QFileDialog>

#include "qzcommon.h"

class QSslCertificate;
class QFontMetrics;
class QWebFrame;
class QPixmap;
class QIcon;
class QWidget;
class QUrl;

class QUPZILLA_EXPORT QzTools
{
public:
    static QByteArray pixmapToByteArray(const QPixmap &pix);
    static QPixmap pixmapFromByteArray(const QByteArray &data);

    static QString readAllFileContents(const QString &filename);
    static QByteArray readAllFileByteContents(const QString &filename);

    static void centerWidgetOnScreen(QWidget* w);
    static void centerWidgetToParent(QWidget* w, QWidget* parent);

    static bool removeFile(const QString &fullFileName);
    static void removeDir(const QString &d);

    static QString samePartOfStrings(const QString &one, const QString &other);
    static QString urlEncodeQueryString(const QUrl &url);
    static QString fromPunycode(const QString &str);
    static QString escapeSqlString(QString urlString);
    static QUrl frameUrl(QWebFrame* frame);

    static QString ensureUniqueFilename(const QString &name, const QString &appendFormat = QString("(%1)"));
    static QString getFileNameFromUrl(const QUrl &url);
    static QString filterCharsFromFilename(const QString &name);

    static QString lastPathForFileDialog(const QString &dialogName, const QString &fallbackPath);
    static void saveLastPathForFileDialog(const QString &dialogName, const QString &path);

    static QString alignTextToWidth(const QString &string, const QString &text, const QFontMetrics &metrics, int width);
    static QString fileSizeToString(qint64 size);

    static QPixmap createPixmapForSite(const QIcon &icon, const QString &title, const QString &url);
    static QString applyDirectionToPage(QString &pageContents);
    static QString truncatedText(const QString &text, int size);

    static QString resolveFromPath(const QString &name);
    static QStringList splitCommandArguments(const QString &command);
    static bool startExternalProcess(const QString &executable, const QString &args);

    static QRegion roundedRect(const QRect &rect, int radius);
    static QIcon iconFromFileName(const QString &fileName);
    static bool isUtf8(const char* string);

    static bool containsSpace(const QString &str);

    // QFileDialog static functions that remembers last used directory
    static QString getExistingDirectory(const QString &name, QWidget* parent = 0, const QString &caption = QString(), const QString &dir = QString(), QFileDialog::Options options = QFileDialog::ShowDirsOnly);
    static QString getOpenFileName(const QString &name, QWidget* parent = 0, const QString &caption = QString(), const QString &dir = QString(), const QString &filter = QString(), QString* selectedFilter = 0, QFileDialog::Options options = 0);
    static QStringList getOpenFileNames(const QString &name, QWidget* parent = 0, const QString &caption = QString(), const QString &dir = QString(), const QString &filter = QString(), QString* selectedFilter = 0, QFileDialog::Options options = 0);
    static QString getSaveFileName(const QString &name, QWidget* parent = 0, const QString &caption = QString(), const QString &dir = QString(), const QString &filter = QString(), QString* selectedFilter = 0, QFileDialog::Options options = 0);

    static bool matchDomain(const QString &pattern, const QString &domain);

    static QString operatingSystem();

    // Qt5 migration help functions
    static bool isCertificateValid(const QSslCertificate &cert);
    static QString escape(const QString &string);

    static bool isPlatformX11();
    static void setWmClass(const QString &name, const QWidget* widget);

    template <typename T>
    static bool containsIndex(const T &container, int index)
    {
        return (index >= 0 && container.count() > index);
    }
};

#endif // QZTOOLS_H
