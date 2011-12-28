/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca <nowrep@gmail.com>
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
#ifndef COMMANDLINEOPTIONS_H
#define COMMANDLINEOPTIONS_H

#include <QObject>
#include <QPair>
#include <iostream>

class CommandLineOptions : public QObject
{
    Q_OBJECT
public:
    enum Action { NoAction, OpenUrl, StartWithProfile, StartWithoutAddons,
                  NewTab, NewWindow, ShowDownloadManager, StartPrivateBrowsing,
                  ExitAction
                };

    struct ActionPair {
        Action action;
        QString text;
    };

    explicit CommandLineOptions(int &argc, char** argv);
    QList<ActionPair> getActions() { return m_actions; }

private:
    void showHelp();
    void parseActions();

    int m_argc;
    char** m_argv;
    QList<ActionPair> m_actions;
};

#endif // COMMANDLINEOPTIONS_H
