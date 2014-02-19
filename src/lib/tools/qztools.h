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
#ifndef GLOBALFUNCTIONS_H
#define GLOBALFUNCTIONS_H

#include <QList>
#include <QString>
#include <QRegion>
#include <QFileDialog>

#include "qz_namespace.h"

class QSslCertificate;
class QFontMetrics;
class QPixmap;
class QIcon;
class QWidget;
class QUrl;

namespace QzTools
{
QByteArray QUPZILLA_EXPORT pixmapToByteArray(const QPixmap &pix);
QPixmap QUPZILLA_EXPORT pixmapFromByteArray(const QByteArray &data);

QString QUPZILLA_EXPORT readAllFileContents(const QString &filename);
QByteArray QUPZILLA_EXPORT readAllFileByteContents(const QString &filename);

void QUPZILLA_EXPORT centerWidgetOnScreen(QWidget* w);
void QUPZILLA_EXPORT centerWidgetToParent(QWidget* w, QWidget* parent);

bool QUPZILLA_EXPORT removeFile(const QString &fullFileName);
void QUPZILLA_EXPORT removeDir(const QString &d);

QString QUPZILLA_EXPORT samePartOfStrings(const QString &one, const QString &other);
QString QUPZILLA_EXPORT urlEncodeQueryString(const QUrl &url);

QString QUPZILLA_EXPORT ensureUniqueFilename(const QString &name, const QString &appendFormat = QString("(%1)"));
QString QUPZILLA_EXPORT getFileNameFromUrl(const QUrl &url);
QString QUPZILLA_EXPORT filterCharsFromFilename(const QString &name);

QString QUPZILLA_EXPORT lastPathForFileDialog(const QString &dialogName, const QString &fallbackPath);
void QUPZILLA_EXPORT saveLastPathForFileDialog(const QString &dialogName, const QString &path);

QString QUPZILLA_EXPORT alignTextToWidth(const QString &string, const QString &text, const QFontMetrics &metrics, int width);
QString QUPZILLA_EXPORT fileSizeToString(qint64 size);

QPixmap QUPZILLA_EXPORT createPixmapForSite(const QIcon &icon, const QString &title, const QString &url);
QString QUPZILLA_EXPORT applyDirectionToPage(QString &pageContents);

QString QUPZILLA_EXPORT resolveFromPath(const QString &name);
QStringList QUPZILLA_EXPORT splitCommandArguments(const QString &command);
bool QUPZILLA_EXPORT startExternalProcess(const QString &executable, const QString &args);

QRegion QUPZILLA_EXPORT roundedRect(const QRect &rect, int radius);
QIcon QUPZILLA_EXPORT iconFromFileName(const QString &fileName);
bool QUPZILLA_EXPORT isUtf8(const char* string);

// QFileDialog static functions that remembers last used directory
QString QUPZILLA_EXPORT getExistingDirectory(const QString &name, QWidget* parent = 0, const QString &caption = QString(), const QString &dir = QString(), QFileDialog::Options options = QFileDialog::ShowDirsOnly);
QString QUPZILLA_EXPORT getOpenFileName(const QString &name, QWidget* parent = 0, const QString &caption = QString(), const QString &dir = QString(), const QString &filter = QString(), QString* selectedFilter = 0, QFileDialog::Options options = 0);
QStringList QUPZILLA_EXPORT getOpenFileNames(const QString &name, QWidget* parent = 0, const QString &caption = QString(), const QString &dir = QString(), const QString &filter = QString(), QString* selectedFilter = 0, QFileDialog::Options options = 0);
QString QUPZILLA_EXPORT getSaveFileName(const QString &name, QWidget* parent = 0, const QString &caption = QString(), const QString &dir = QString(), const QString &filter = QString(), QString* selectedFilter = 0, QFileDialog::Options options = 0);

bool QUPZILLA_EXPORT matchDomain(const QString &pattern, const QString &domain);

QString QUPZILLA_EXPORT operatingSystem();

// Qt5 migration help functions
bool QUPZILLA_EXPORT isCertificateValid(const QSslCertificate &cert);
QString QUPZILLA_EXPORT escape(const QString &string);

#if defined(QZ_WS_X11) && !defined(NO_X11)
void* X11Display(const QWidget* widget);
#endif

void QUPZILLA_EXPORT setWmClass(const QString &name, const QWidget* widget);

template <typename T>
bool listContainsIndex(const QList<T> &list, int index)
{
    return (index >= 0 && list.count() > index);
}

template <typename T>
bool vectorContainsIndex(const QVector<T> &list, int index)
{
    return (index >= 0 && list.count() > index);
}

} // namespace

#endif // GLOBALFUNCTIONS_H
