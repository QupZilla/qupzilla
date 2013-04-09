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
#ifndef COMMANDLINEOPTIONS_H
#define COMMANDLINEOPTIONS_H

#include <QPair>
#include <QString>
#include <QVector>
#include <iostream>

#include "qz_namespace.h"

class QT_QUPZILLA_EXPORT CommandLineOptions
{
public:
    struct ActionPair {
        Qz::CommandLineAction action;
        QString text;
    };

    typedef QVector<ActionPair> ActionPairList;

    explicit CommandLineOptions(int &argc);
    ActionPairList getActions();

private:
    void showHelp();
    void parseActions();

    int m_argc;
    ActionPairList m_actions;
};

// Hint to QVector to use std::realloc on item moving
Q_DECLARE_TYPEINFO(CommandLineOptions::ActionPair, Q_MOVABLE_TYPE);

#endif // COMMANDLINEOPTIONS_H
