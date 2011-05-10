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
#include "commandlineoptions.h"
#include "qupzilla.h"

CommandLineOptions::CommandLineOptions(int &argc, char **argv) :
  QObject(0)
  ,m_argc(argc)
  ,m_argv(argv)
{
    parseActions();
}

void CommandLineOptions::showHelp()
{
    using namespace std;

    const char* help= " Usage: qupzilla [options] URL  \n"
                      "\n"
                      " QupZilla options:\n"
                      "    -h or -help                  print this message \n"
                      "    -a or -authors               print QupZilla authors \n"
                      "    -v or -version               print QupZilla version \n"
                      "    -p or -profile=PROFILE       start with specified profile \n"
                      "    -np or -no-plugins           start without plugins \n"
                      "\n"
                      " QupZilla is a new, fast and secure web browser\n"
                      " based on WebKit core (http://webkit.org) and\n"
                      " written in Qt Framework (http://qt.nokia.com) \n\n"
                      " For more informations please visit wiki at \n"
                      " https://github.com/nowrep/QupZilla/wiki \n"
                      ;
    cout << help << " > " << QupZilla::WWWADDRESS.toAscii().data() << endl;
}

void CommandLineOptions::parseActions()
{
    using namespace std;

    bool found = false;
    // Skip first argument (it should be program itself)
    for (int i = 1; i < m_argc; i++) {
        QString arg(m_argv[i]);
        if (arg == "-h" || arg == "-help") {
            showHelp();
            found = true;
            break;
        }
        if (arg == "-a" || arg == "-authors") {
            cout << "QupZilla authors: " << endl;
            cout << "  nowrep <nowrep@gmail.com>" << endl;
            found = true;
            break;
        }
        if (arg == "-v" || arg == "-version") {
            cout << "QupZilla v" << QupZilla::VERSION.toAscii().data()
                 << "(build " << QupZilla::BUILDTIME.toAscii().data() << ")"
                 << endl;
            found = true;
            break;
        }

        if (arg.startsWith("-p=") || arg.startsWith("-profile=")) {
            arg.remove("-p=");
            arg.remove("-profile=");
            found = true;
            cout << "starting with profile " << arg.toAscii().data() << endl;
            QPair<int, QString> pair;
            pair.first = StartWithProfile;
            pair.second = arg;
            m_actions.append(pair);
        }

        if (arg.startsWith("-np") || arg.startsWith("-no-plugins")) {
            found = true;
            QPair<int, QString> pair;
            pair.first = StartWithoutAddons;
            pair.second = "";
            m_actions.append(pair);
        }
    }

    QString url(m_argv[m_argc-1]);
    if (m_argc > 1 && !url.isEmpty() && !url.startsWith("-")) {
        found = true;
        cout << "starting with url " << url.toAscii().data() << endl;
        QPair<int, QString> pair;
        pair.first = OpenUrl;
        pair.second = url;
        m_actions.append(pair);
    }

    if (m_argc > 1 && !found) {
        cout << "bad arguments!" << endl;
        showHelp();
    }

}
