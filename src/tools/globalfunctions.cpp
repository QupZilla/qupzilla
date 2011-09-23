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
#include "globalfunctions.h"

QByteArray qz_pixmapToByteArray(const QPixmap &pix)
{
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    if (pix.save(&buffer, "PNG"))
        return buffer.buffer().toBase64();

    return QByteArray();
}

QByteArray qz_readAllFileContents(const QString &filename)
{
    QFile file(filename);
    file.open(QFile::ReadOnly);
    QByteArray a = file.readAll();
    file.close();

    return a;
}

void qz_centerWidgetOnScreen(QWidget *w)
{
    const QRect screen = QApplication::desktop()->screenGeometry();
    const QRect &size = w->geometry();
    w->move( (screen.width()-size.width())/2, (screen.height()-size.height())/2 );
}
