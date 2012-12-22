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
#ifndef GLOBALFUNCTIONS_H
#define GLOBALFUNCTIONS_H

#include <QList>
#include <QString>

#include "qz_namespace.h"

class QSslCertificate;
class QFontMetrics;
class QPixmap;
class QIcon;
class QWidget;
class QUrl;

QByteArray QT_QUPZILLA_EXPORT qz_pixmapToByteArray(const QPixmap &pix);
QPixmap QT_QUPZILLA_EXPORT qz_pixmapFromByteArray(const QByteArray &data);

QString QT_QUPZILLA_EXPORT qz_readAllFileContents(const QString &filename);

void QT_QUPZILLA_EXPORT qz_centerWidgetOnScreen(QWidget* w);
void QT_QUPZILLA_EXPORT qz_centerWidgetToParent(QWidget* w, QWidget* parent);

bool QT_QUPZILLA_EXPORT qz_removeFile(const QString &fullFileName);
void QT_QUPZILLA_EXPORT qz_removeDir(const QString &d);

QString QT_QUPZILLA_EXPORT qz_samePartOfStrings(const QString &one, const QString &other);
QUrl QT_QUPZILLA_EXPORT qz_makeRelativeUrl(const QUrl &baseUrl, const QUrl &rUrl);
QString QT_QUPZILLA_EXPORT qz_urlEncodeQueryString(const QUrl &url);

QString QT_QUPZILLA_EXPORT qz_ensureUniqueFilename(const QString &name, const QString &appendFormat = QString("(%1)"));
QString QT_QUPZILLA_EXPORT qz_getFileNameFromUrl(const QUrl &url);
QString QT_QUPZILLA_EXPORT qz_filterCharsFromFilename(const QString &name);

QString QT_QUPZILLA_EXPORT qz_alignTextToWidth(const QString &string, const QString &text, const QFontMetrics &metrics, int width);
QString QT_QUPZILLA_EXPORT qz_fileSizeToString(qint64 size);

QPixmap QT_QUPZILLA_EXPORT qz_createPixmapForSite(const QIcon &icon, const QString &title, const QString &url);
QString QT_QUPZILLA_EXPORT qz_applyDirectionToPage(QString &pageContents);

QString QT_QUPZILLA_EXPORT qz_buildSystem();

// Qt5 migration help functions
bool QT_QUPZILLA_EXPORT qz_isCertificateValid(const QSslCertificate &cert);
QString QT_QUPZILLA_EXPORT qz_escape(const QString &string);

#ifdef QZ_WS_X11
void QT_QUPZILLA_EXPORT* qz_X11Display(const QWidget* widget);
#endif

template <typename T>
bool qz_listContainsIndex(const QList<T> &list, int index)
{
    return (index >= 0 && list.count() > index);
}

#endif // GLOBALFUNCTIONS_H
