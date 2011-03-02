#ifndef COMMANDLINEOPTIONS_H
#define COMMANDLINEOPTIONS_H

#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

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
