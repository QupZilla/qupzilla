/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  nowrep
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

#include <QByteArray>
#include <QPixmap>
#include <QBuffer>
#include <QFile>
#include <QWidget>
#include <QApplication>
#include <QDesktopWidget>
#include <QUrl>

QByteArray qz_pixmapToByteArray(const QPixmap &pix);
QByteArray qz_readAllFileContents(const QString &filename);

void qz_centerWidgetOnScreen(QWidget* w);
QString qz_samePartOfStrings(const QString &one, const QString &other);
QUrl qz_makeRelativeUrl(const QUrl &baseUrl, const QUrl &rUrl);

QString qz_ensureUniqueFilename(const QString &name);

QString qz_buildSystem();

#endif // GLOBALFUNCTIONS_H
