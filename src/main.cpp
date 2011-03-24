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

#include <QtGui/QApplication>
#include <QTextCodec>
#include <QtPlugin>
#include <iostream>
#include "mainapplication.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(icons);
    Q_INIT_RESOURCE(html);

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#ifdef Q_WS_X11
    QApplication::setGraphicsSystem("raster"); // Better overall performance on X11
#endif

    MainApplication app(argc, argv);
    if (app.isExited()) {
        std::cout << "QupZilla already running - activating existing window" << std::endl;
        return 1;
    }
    return app.exec();
}
