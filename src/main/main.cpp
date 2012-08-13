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
#include "mainapplication.h"
#include "proxystyle.h"

#include <QMessageBox> // For QT_REQUIRE_VERSION

#ifdef Q_OS_LINUX
#include <iostream>
#include <signal.h>
#include <execinfo.h>

#include "qupzilla.h"

#include <QDir>
#include <QDateTime>
#include <QTextStream>

void qupzilla_signal_handler(int s)
{
    switch (s) {
    case SIGPIPE:
        // When using QtWebKit 2.2 and running QupZilla with gdb, I have experienced
        // some SIGPIPEs so far (not with other versions, only with this).
        // But every time, QupZilla was running fine after SIGPIPE, so we are catching
        // this signal and ignoring it to prevent unneeded crash because of it.

        std::cout << "QupZilla: Caught SIGPIPE!" << std::endl;
        break;

    case SIGSEGV: {
        static bool sigSegvServed = false;
        if (sigSegvServed) {
            abort();
        }
        sigSegvServed = true;

        std::cout << "QupZilla: Crashed :( Saving backtrace in " << qPrintable(mApp->PROFILEDIR) << "crashlog ..." << std::endl;

        void* array[100];
        int size = backtrace(array, 100);
        char** strings = backtrace_symbols(array, size);

        if (size < 0 || !strings) {
            std::cout << "Cannot get backtrace!" << std::endl;
            abort();
        }

        QDir dir(mApp->PROFILEDIR);
        if (!dir.exists()) {
            std::cout << qPrintable(mApp->PROFILEDIR) << " does not exist" << std::endl;
            abort();
        }

        if (!dir.cd("crashlog")) {
            if (!dir.mkdir("crashlog")) {
                std::cout << "Cannot create " << qPrintable(mApp->PROFILEDIR) << "crashlog directory!" << std::endl;
                abort();
            }

            dir.cd("crashlog");
        }

        const QDateTime &currentDateTime = QDateTime::currentDateTime();

        QFile file(dir.absoluteFilePath("Crash-" + currentDateTime.toString(Qt::ISODate) + ".txt"));
        if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
            std::cout << "Cannot open file " << qPrintable(file.fileName()) << " for writing!" << std::endl;
            abort();
        }

        QTextStream stream(&file);
        stream << "Time: " << currentDateTime.toString() << endl;
        stream << "Qt version: " << qVersion() << " (compiled with " << QT_VERSION_STR << ")" << endl;
        stream << "QupZilla version: " << QupZilla::VERSION << endl;
        stream << "WebKit version: " << qWebKitVersion() << endl;
        stream << endl;
        stream << "============== BACKTRACE ==============" << endl;

        for (int i = 0; i < size; ++i) {
            stream << "#" << i << ": " << strings[i] << endl;
        }

        file.close();

        std::cout << "Backtrace successfuly saved in " << qPrintable(dir.absoluteFilePath(file.fileName())) << std::endl;
    }

    default:
        break;
    }
}
#endif

int main(int argc, char* argv[])
{
    QT_REQUIRE_VERSION(argc, argv, "4.7.0");

#ifdef Q_WS_X11
    QApplication::setGraphicsSystem("raster"); // Better overall performance on X11
#endif

#ifdef Q_OS_LINUX
    signal(SIGSEGV, qupzilla_signal_handler);
    signal(SIGPIPE, qupzilla_signal_handler);
#endif

    MainApplication app(argc, argv);

    if (app.isClosing()) {
//        Not showing any output, otherwise XFCE shows "Failed to execute default browser. I/O error" error
//        if (argc == 1) {
//            std::cout << "QupZilla already running - activating existing window" << std::endl;
//        }
        return 0;
    }

    app.setStyle(new ProxyStyle);

    return app.exec();
}
