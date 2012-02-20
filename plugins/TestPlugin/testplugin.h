#ifndef TESTPLUGIN_H
#define TESTPLUGIN_H

//Include actual plugininterface.h for your version of QupZilla
//This file is available to download at QupZilla website

#include "plugininterface.h"

//For clean plugin directory, please build necessary files into
//plugin in .qrc data files

#include <QDebug>
#include <QLabel>
#include <QMessageBox>
#include <QWebElement>
#include <QVBoxLayout>

class TestPlugin : public QObject, public PluginInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginInterface)

public:
    PluginSpec pluginSpec();

    void init(const QString &sPath);
    void unload();
    bool testPlugin();

    QTranslator* getTranslator(const QString &locale);
    void showSettings(QWidget* parent = 0);

    void populateWebViewMenu(QMenu* menu, WebView* view, const QWebHitTestResult &r);

private slots:
    void actionSlot();

private:
    WebView* m_view;
    QString m_settingsPath;
};

#endif // TESTPLUGIN_H
