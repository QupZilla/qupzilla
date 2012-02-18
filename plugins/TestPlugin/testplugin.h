#ifndef TESTPLUGIN_H
#define TESTPLUGIN_H

//Include actual plugininterface.h for your version of QupZilla
//This file is available to download at QupZilla website

#include "plugininterface.h"
#include "historymodel.h"


//For clean plugin directory, please build necessary files into
//plugin in .qrc data files

#include <QDebug>
#include <QLabel>
#include <QMessageBox>
#include <QWebElement>

class TestPlugin : public QObject, public PluginInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginInterface)

public:
    QString pluginName() { return tr("Example Plugin"); }
    QString pluginInfo() { return tr("Example minimal plugin"); }
    QString pluginDescription() { return tr("Very simple minimal plugin example"); }
    QString pluginVersion() { return "0.0.1"; }
    QString pluginAuthor() { return "David Rosca <nowrep@gmail.com>"; }
    void init(QString sPath);
    bool testPlugin();

    QTranslator* getTranslator(QString locale);
    QIcon pluginIcon() { return QIcon(":/qupzilla.png"); }
    bool hasSettings() { return true; }
    void showSettings();

    void populateWebViewMenu(QMenu* menu, QWebView* view, QWebHitTestResult r);
    void populateHelpMenu(QMenu* menu);
    void populateToolsMenu(QMenu* menu);

private slots:
    void actionSlot();
private:
    QString settingsPath;
};

#endif // TESTPLUGIN_H
