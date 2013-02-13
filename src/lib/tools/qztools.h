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

namespace QzTools
{
QByteArray QT_QUPZILLA_EXPORT pixmapToByteArray(const QPixmap &pix);
QPixmap QT_QUPZILLA_EXPORT pixmapFromByteArray(const QByteArray &data);

QString QT_QUPZILLA_EXPORT readAllFileContents(const QString &filename);

void QT_QUPZILLA_EXPORT centerWidgetOnScreen(QWidget* w);
void QT_QUPZILLA_EXPORT centerWidgetToParent(QWidget* w, QWidget* parent);

bool QT_QUPZILLA_EXPORT removeFile(const QString &fullFileName);
void QT_QUPZILLA_EXPORT removeDir(const QString &d);

QString QT_QUPZILLA_EXPORT samePartOfStrings(const QString &one, const QString &other);
QString QT_QUPZILLA_EXPORT urlEncodeQueryString(const QUrl &url);

QString QT_QUPZILLA_EXPORT ensureUniqueFilename(const QString &name, const QString &appendFormat = QString("(%1)"));
QString QT_QUPZILLA_EXPORT getFileNameFromUrl(const QUrl &url);
QString QT_QUPZILLA_EXPORT filterCharsFromFilename(const QString &name);

QString QT_QUPZILLA_EXPORT alignTextToWidth(const QString &string, const QString &text, const QFontMetrics &metrics, int width);
QString QT_QUPZILLA_EXPORT fileSizeToString(qint64 size);

QPixmap QT_QUPZILLA_EXPORT createPixmapForSite(const QIcon &icon, const QString &title, const QString &url);
QString QT_QUPZILLA_EXPORT applyDirectionToPage(QString &pageContents);

QString QT_QUPZILLA_EXPORT resolveFromPath(const QString &name);

QIcon QT_QUPZILLA_EXPORT iconFromFileName(const QString &fileName);

QString QT_QUPZILLA_EXPORT buildSystem();

// Qt5 migration help functions
bool QT_QUPZILLA_EXPORT isCertificateValid(const QSslCertificate &cert);
QString QT_QUPZILLA_EXPORT escape(const QString &string);

#ifdef QZ_WS_X11
void QT_QUPZILLA_EXPORT* X11Display(const QWidget* widget);
#endif

template <typename T>
bool listContainsIndex(const QList<T> &list, int index)
{
    return (index >= 0 && list.count() > index);
}

} // namespace

#endif // GLOBALFUNCTIONS_H
