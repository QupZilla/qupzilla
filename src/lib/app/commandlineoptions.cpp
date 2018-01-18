/* ============================================================
* QupZilla - QtWebEngine based browser
* Copyright (C) 2010-2018 David Rosca <nowrep@gmail.com>
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

#include <QFileInfo>
#include <QCoreApplication>
#include <QCommandLineParser>

#include <iostream>

CommandLineOptions::CommandLineOptions()
{
    parseActions();
}

CommandLineOptions::ActionPairList CommandLineOptions::getActions()
{
    return m_actions;
}

void CommandLineOptions::parseActions()
{
    // Options
    QCommandLineOption authorsOption(QStringList({QSL("a"), QSL("authors")}));
    authorsOption.setDescription(QSL("Displays author information."));

    QCommandLineOption profileOption(QStringList({QSL("p"), QSL("profile")}));
    profileOption.setValueName(QSL("profileName"));
    profileOption.setDescription(QSL("Starts with specified profile."));

    QCommandLineOption noExtensionsOption(QStringList({QSL("e"), QSL("no-extensions")}));
    noExtensionsOption.setDescription(QSL("Starts without extensions."));

    QCommandLineOption privateBrowsingOption(QStringList({QSL("i"), QSL("private-browsing")}));
    privateBrowsingOption.setDescription(QSL("Starts private browsing."));

    QCommandLineOption portableOption(QStringList({QSL("o"), QSL("portable")}));
    portableOption.setDescription(QSL("Starts in portable mode."));

    QCommandLineOption noRemoteOption(QStringList({QSL("r"), QSL("no-remote")}));
    noRemoteOption.setDescription(QSL("Starts new browser instance."));

    QCommandLineOption newTabOption(QStringList({QSL("t"), QSL("new-tab")}));
    newTabOption.setDescription(QSL("Opens new tab."));

    QCommandLineOption newWindowOption(QStringList({QSL("w"), QSL("new-window")}));
    newWindowOption.setDescription(QSL("Opens new window."));

    QCommandLineOption downloadManagerOption(QStringList({QSL("d"), QSL("download-manager")}));
    downloadManagerOption.setDescription(QSL("Opens download manager."));

    QCommandLineOption currentTabOption(QStringList({QSL("c"), QSL("current-tab")}));
    currentTabOption.setValueName(QSL("URL"));
    currentTabOption.setDescription(QSL("Opens URL in current tab."));

    QCommandLineOption openWindowOption(QStringList({QSL("u"), QSL("open-window")}));
    openWindowOption.setValueName(QSL("URL"));
    openWindowOption.setDescription(QSL("Opens URL in new window."));

    QCommandLineOption fullscreenOption(QStringList({QSL("f"), QSL("fullscreen")}));
    fullscreenOption.setDescription(QSL("Toggles fullscreen."));

    // Parser
    QCommandLineParser parser;
    parser.setApplicationDescription(QSL("QtWebEngine based browser"));
    QCommandLineOption helpOption = parser.addHelpOption();
    QCommandLineOption versionOption = parser.addVersionOption();
    parser.addOption(authorsOption);
    parser.addOption(profileOption);
    parser.addOption(noExtensionsOption);
    parser.addOption(privateBrowsingOption);
    parser.addOption(portableOption);
    parser.addOption(noRemoteOption);
    parser.addOption(newTabOption);
    parser.addOption(newWindowOption);
    parser.addOption(downloadManagerOption);
    parser.addOption(currentTabOption);
    parser.addOption(openWindowOption);
    parser.addOption(fullscreenOption);
    parser.addPositionalArgument(QSL("URL"), QSL("URLs to open"), QSL("[URL...]"));

    // parse() and not process() so we can pass arbitrary options to Chromium
    parser.parse(QCoreApplication::arguments());

    if (parser.isSet(helpOption)) {
        parser.showHelp();
    }

    if (parser.isSet(versionOption)) {
        parser.showVersion();
    }

    if (parser.isSet(authorsOption)) {
        std::cout << "David Rosca <nowrep@gmail.com>" << std::endl;

        ActionPair pair;
        pair.action = Qz::CL_ExitAction;
        m_actions.append(pair);
        return;
    }

    if (parser.isSet(profileOption)) {
        const QString profileName = parser.value(profileOption);
        std::cout << "QupZilla: Starting with profile '" << profileName.toUtf8().data() << "'" << std::endl;

        ActionPair pair;
        pair.action = Qz::CL_StartWithProfile;
        pair.text = profileName;
        m_actions.append(pair);
    }

    if (parser.isSet(noExtensionsOption)) {
        ActionPair pair;
        pair.action = Qz::CL_StartWithoutAddons;
        m_actions.append(pair);
    }

    if (parser.isSet(privateBrowsingOption)) {
        ActionPair pair;
        pair.action = Qz::CL_StartPrivateBrowsing;
        m_actions.append(pair);
    }

    if (parser.isSet(portableOption)) {
        ActionPair pair;
        pair.action = Qz::CL_StartPortable;
        m_actions.append(pair);
    }

    if (parser.isSet(noRemoteOption)) {
        ActionPair pair;
        pair.action = Qz::CL_StartNewInstance;
        m_actions.append(pair);
    }

    if (parser.isSet(newTabOption)) {
        ActionPair pair;
        pair.action = Qz::CL_NewTab;
        m_actions.append(pair);
    }

    if (parser.isSet(newWindowOption)) {
        ActionPair pair;
        pair.action = Qz::CL_NewWindow;
        m_actions.append(pair);
    }

    if (parser.isSet(downloadManagerOption)) {
        ActionPair pair;
        pair.action = Qz::CL_ShowDownloadManager;
        m_actions.append(pair);
    }

    if (parser.isSet(currentTabOption)) {
        ActionPair pair;
        pair.action = Qz::CL_OpenUrlInCurrentTab;
        pair.text = parser.value(currentTabOption);
        m_actions.append(pair);
    }

    if (parser.isSet(openWindowOption)) {
        ActionPair pair;
        pair.action = Qz::CL_OpenUrlInNewWindow;
        pair.text = parser.value(openWindowOption);
        m_actions.append(pair);
    }

    if (parser.isSet(fullscreenOption)) {
        ActionPair pair;
        pair.action = Qz::CL_ToggleFullScreen;
        m_actions.append(pair);
    }

    if (parser.positionalArguments().isEmpty())
        return;

    QString url = parser.positionalArguments().last();
    QFileInfo fileInfo(url);

    if (fileInfo.exists()) {
        url = fileInfo.absoluteFilePath();
    }

    if (!url.isEmpty() && !url.startsWith(QLatin1Char('-'))) {
        ActionPair pair;
        pair.action = Qz::CL_OpenUrl;
        pair.text = url;
        m_actions.append(pair);
    }
}
