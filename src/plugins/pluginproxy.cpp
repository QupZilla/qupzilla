#include "pluginproxy.h"
#include "plugininterface.h"
#include "mainapplication.h"

PluginProxy::PluginProxy() :
    Plugins()
{
    c2f_loadSettings();
}

void PluginProxy::populateWebViewMenu(QMenu *menu, QWebView *view, QWebHitTestResult r)
{
    if (!menu || !view || loadedPlugins.count() == 0)
        return;

    menu->addSeparator();
    int count = menu->actions().count();

    foreach(PluginInterface* iPlugin, loadedPlugins)
        iPlugin->populateWebViewMenu(menu, view, r);

    if (menu->actions().count() == count)
        menu->removeAction(menu->actions().at(count));
}

void PluginProxy::populateToolsMenu(QMenu *menu)
{
    if (!menu || loadedPlugins.count() == 0)
        return;

    int count = menu->actions().count();

    foreach(PluginInterface* iPlugin, loadedPlugins)
        iPlugin->populateToolsMenu(menu);

    if (menu->actions().count() != count)
        menu->addSeparator();
}

void PluginProxy::populateHelpMenu(QMenu *menu)
{
    if (!menu || loadedPlugins.count() == 0)
        return;

    int count = menu->actions().count();

    foreach(PluginInterface* iPlugin, loadedPlugins)
        iPlugin->populateHelpMenu(menu);

    if (menu->actions().count() != count)
        menu->addSeparator();
}

void PluginProxy::c2f_loadSettings()
{
    QSettings settings(MainApplication::getInstance()->getActiveProfil()+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("ClickToFlash");
    c2f_whitelist = settings.value("whitelist", QStringList()).toStringList();
    c2f_enabled = settings.value("Enabled", true).toBool();
    settings.endGroup();
}

void PluginProxy::c2f_saveSettings()
{
    QSettings settings(MainApplication::getInstance()->getActiveProfil()+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("ClickToFlash");
    settings.setValue("whitelist", c2f_whitelist);
    settings.setValue("Enabled", c2f_enabled);
    settings.endGroup();
}
