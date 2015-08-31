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
#include "mainapplication.h"
#include "webinspector.h"
#include "proxystyle.h"
#include "datapaths.h"

#include <QMessageBox> // For QT_REQUIRE_VERSION
#include <iostream>
#include <qtwebengineglobal.h>

#if defined(Q_OS_LINUX) || defined(__GLIBC__) || defined(__FreeBSD__) || defined(__HAIKU__)
#include <signal.h>
#include <execinfo.h>

#include <QDir>
#include <QDateTime>
#include <QTextStream>
#include <QWebEnginePage>

void qupzilla_signal_handler(int s)
{
    if (s != SIGSEGV) {
        return;
    }

    static bool sigSegvServed = false;
    if (sigSegvServed) {
        abort();
    }
    sigSegvServed = true;

    std::cout << "QupZilla: Crashed :( Saving backtrace in " << qPrintable(DataPaths::path(DataPaths::Config)) << "/crashlog ..." << std::endl;

    void* array[100];
    int size = backtrace(array, 100);
    char** strings = backtrace_symbols(array, size);

    if (size < 0 || !strings) {
        std::cout << "Cannot get backtrace!" << std::endl;
        abort();
    }

    QDir dir(DataPaths::path(DataPaths::Config));
    if (!dir.exists()) {
        std::cout << qPrintable(DataPaths::path(DataPaths::Config)) << " does not exist" << std::endl;
        abort();
    }

    if (!dir.cd("crashlog")) {
        if (!dir.mkdir("crashlog")) {
            std::cout << "Cannot create " << qPrintable(DataPaths::path(DataPaths::Config)) << "crashlog directory!" << std::endl;
            abort();
        }

        dir.cd("crashlog");
    }

    const QDateTime currentDateTime = QDateTime::currentDateTime();

    QFile file(dir.absoluteFilePath("Crash-" + currentDateTime.toString(Qt::ISODate) + ".txt"));
    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        std::cout << "Cannot open file " << qPrintable(file.fileName()) << " for writing!" << std::endl;
        abort();
    }

    QTextStream stream(&file);
    stream << "Time: " << currentDateTime.toString() << endl;
    stream << "Qt version: " << qVersion() << " (compiled with " << QT_VERSION_STR << ")" << endl;
    stream << "QupZilla version: " << Qz::VERSION << endl;
    stream << "Rendering engine: QtWebEngine" << endl;
    stream << endl;
    stream << "============== BACKTRACE ==============" << endl;

    for (int i = 0; i < size; ++i) {
        stream << "#" << i << ": " << strings[i] << endl;
    }

    file.close();

    std::cout << "Backtrace successfuly saved in " << qPrintable(dir.absoluteFilePath(file.fileName())) << std::endl;
}
#endif // defined(Q_OS_LINUX) || defined(__GLIBC__) || defined(__FreeBSD__)

#ifndef Q_OS_WIN
void msgHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (msg.startsWith(QL1S("QSslSocket: cannot resolve SSLv2_")))
        return;
    if (msg.startsWith(QL1S("Remote debugging server started successfully.")))
        return;

    const QByteArray localMsg = msg.toLocal8Bit();

    switch (type) {
    case QtDebugMsg:
    case QtWarningMsg:
    case QtCriticalMsg:
        std::cerr << localMsg.constData();
        if (context.file && context.line && context.function)
            std::cerr << " (" <<  context.file << ":" << context.line << ", " << context.function << ")";
        std::cerr << std::endl;
        break;

    case QtFatalMsg:
        std::cerr << "Fatal: " << localMsg.constData() << std::endl;
        if (context.file && context.line && context.function)
            std::cerr << " (" <<  context.file << ":" << context.line << ", " << context.function << ")";
        std::cerr << std::endl;
        abort();

    default:
        break;
    }
}
#endif

int main(int argc, char* argv[])
{
    QT_REQUIRE_VERSION(argc, argv, "4.7.0");

#ifndef Q_OS_WIN
    qInstallMessageHandler(&msgHandler);
#endif

#if defined(Q_OS_LINUX) || defined(__GLIBC__) || defined(__FreeBSD__)
    signal(SIGSEGV, qupzilla_signal_handler);
#endif

    // Hack to fix QT_STYLE_OVERRIDE with QProxyStyle
    const QByteArray style = qgetenv("QT_STYLE_OVERRIDE");
    if (!style.isEmpty()) {
        char** args = (char**) malloc(sizeof(char*) * (argc + 1));
        for (int i = 0; i < argc; ++i)
            args[i] = argv[i];

        QString stylecmd = QL1S("-style=") + style;
        args[argc++] = qstrdup(stylecmd.toUtf8().constData());
        argv = args;
    }

    qputenv("QTWEBENGINE_REMOTE_DEBUGGING", WEBINSPECTOR_PORT);

    MainApplication app(argc, argv);

    QtWebEngine::initialize();

    if (app.isClosing())
        return 0;

    app.setProxyStyle(new ProxyStyle);

    return app.exec();
}
