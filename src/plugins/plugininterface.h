#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QtPlugin>
#include <QIcon>
#include <QTranslator>
#include <QMenu>
#include <QNetworkRequest>
#include <QWebView>
#include <QWebHitTestResult>

class PluginInterface
{
public:
    //Plugin Necessary Init Functions
    //You have to reimplement those functions, otherwise QupZilla crash
    virtual ~PluginInterface() { }
    virtual QString pluginName() = 0;
    virtual QString pluginInfo() = 0;
    virtual QString pluginDescription() = 0;
    virtual QString pluginVersion() = 0;
    virtual QString pluginAuthor() = 0;
    virtual void init(QString settingsPath) = 0;
    virtual bool testPlugin() = 0;
    //End Plugin Necessary Init Functions

    virtual QTranslator* getTranslator(QString locale) { Q_UNUSED(locale) return 0; }
    virtual QIcon pluginIcon() { return QIcon(); }
    virtual bool hasSettings() { return false; }
    virtual void showSettings() { }

    virtual void populateToolsMenu(QMenu* menu) { Q_UNUSED(menu) }
    virtual void populateHelpMenu(QMenu* menu) { Q_UNUSED(menu) }
    virtual void populateWebViewMenu(QMenu* menu, QWebView* view, QWebHitTestResult r) { Q_UNUSED(menu) Q_UNUSED(view) Q_UNUSED(r) }

    virtual void formSent(const QNetworkRequest &request, const QByteArray &outgoingData) { Q_UNUSED(request) Q_UNUSED(outgoingData)}
    virtual void pageLoaded(QWebView* view) { Q_UNUSED(view) }
    virtual void downloadRequested(QWidget* requestWidget) { Q_UNUSED(requestWidget) }
};

 Q_DECLARE_INTERFACE(PluginInterface, "Qupzilla.Browser.PluginInterface/1.0")


#endif // PLUGININTERFACE_H
