#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H
#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include <QObject>
#include <QtPlugin>
#include <QPluginLoader>
#include <QDir>
#include <QTimer>
#include <QSettings>
#include <QDebug>

class PluginInterface;
class Plugins : public QObject
{
    Q_OBJECT
public:
    explicit Plugins(QObject *parent = 0);

    QStringList getAvailablePlugins() { return m_availablePluginFileNames; }
    QStringList getAllowedPlugins () { return m_allowedPluginFileNames; }
    PluginInterface* getPlugin(QString pluginFileName);
    //void setPluginsAllowed(bool state) { pluginsEnabled = state; qDebug() << state;}

public slots:
    void loadSettings();
    void loadPlugins();

protected:
    QList<PluginInterface* > loadedPlugins;

private:
    QStringList m_availablePluginFileNames;
    QStringList m_allowedPluginFileNames;
    QStringList m_loadedPluginFileNames;
    bool m_pluginsEnabled;
};

#endif // PLUGINLOADER_H
