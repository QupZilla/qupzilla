#ifndef WEB_PLUGIN_FACTORY_H
#define WEB_PLUGIN_FACTORY_H

#include <QWebPluginFactory>
#include <QDebug>

class WebPluginFactory : public QWebPluginFactory
{
    Q_OBJECT

public:
    WebPluginFactory(QObject *parent);
    virtual QObject*create (const QString &mimeType, const QUrl &url, const QStringList &argumentNames, const QStringList &argumentValues) const;
    QList<QWebPluginFactory::Plugin> plugins() const;

signals:
    void signalLoadClickToFlash(bool) const;

public slots:
    void setLoadClickToFlash(bool load) {  m_loadClickToFlash = load; }

private:
    bool m_loadClickToFlash;
};
#endif // WEB_PLUGIN_FACTORY_H
