#include "pluginproxy.h"
#include "plugininterface.h"
#include "mainapplication.h"

Plugins::Plugins(QObject *parent) :
    QObject(parent)
{
    loadSettings();
}

void Plugins::loadSettings()
{
    m_allowedPluginFileNames.clear();

    QSettings settings(MainApplication::getInstance()->getActiveProfil()+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("Plugin-Settings");
    m_pluginsEnabled = settings.value("EnablePlugins", true).toBool();
    m_allowedPluginFileNames = settings.value("AllowedPlugins", QStringList()).toStringList();
    settings.endGroup();
}

void Plugins::loadPlugins()
{
    if (!m_pluginsEnabled)
        return;

    m_availablePluginFileNames.clear();
    loadedPlugins.clear();

    QDir pluginsDir = QDir(MainApplication::getInstance()->DATADIR+"plugins/");

    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        m_availablePluginFileNames.append(fileName);

        if (!m_allowedPluginFileNames.contains(fileName))
            continue;

        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = loader.instance();
        if (plugin) {
            PluginInterface *iPlugin = qobject_cast<PluginInterface *>(plugin);
            iPlugin->init(MainApplication::getInstance()->getActiveProfil()+"plugins.ini");
            if (!iPlugin->testPlugin()) {
                loader.unload();
                continue;
            }

            qApp->installTranslator(iPlugin->getTranslator(MainApplication::getInstance()->getActiveLanguage()));
            loadedPlugins.append(iPlugin);
            m_loadedPluginFileNames.append(fileName);
        }
    }
    qDebug() << loadedPlugins.count() << "plugins loaded";
}

PluginInterface* Plugins::getPlugin(QString pluginFileName)
{
    QString path = MainApplication::getInstance()->DATADIR+"plugins/"+pluginFileName;
    if (!QFile::exists(path))
        return 0;
    QPluginLoader loader(path);
    QObject *plugin = loader.instance();
    if (plugin) {
        PluginInterface *iPlugin = qobject_cast<PluginInterface *>(plugin);
        return iPlugin;
    } else
        return 0;
}
