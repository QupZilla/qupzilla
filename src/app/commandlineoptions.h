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
#ifndef COMMANDLINEOPTIONS_H
#define COMMANDLINEOPTIONS_H

#include <QObject>

#include <iostream>

class CommandLineOptions : public QObject
{
    Q_OBJECT
public:
    enum Action {NoAction, OpenUrl, StartWithProfile, StartWithoutAddons};
    explicit CommandLineOptions(int &argc, char **argv);
    Action getAction();
    QString getActionString();

private:
    void showHelp();
    void parseActions();

    QString m_actionString;
    int m_argc;
    char **m_argv;
    Action m_action;
};

#endif // COMMANDLINEOPTIONS_H
