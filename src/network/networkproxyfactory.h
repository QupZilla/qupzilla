#ifndef NETWORKPROXYFACTORY_H
#define NETWORKPROXYFACTORY_H

#include <QNetworkProxyFactory>
#include <QUrl>
#include <QStringList>
#include <QSettings>

class NetworkProxyFactory : public QNetworkProxyFactory
{
public:
    enum ProxyPreference { SystemProxy, NoProxy, DefinedProxy };

    explicit NetworkProxyFactory();
    void loadSettings();

    QList<QNetworkProxy> queryProxy(const QNetworkProxyQuery &query = QNetworkProxyQuery());

private:
    ProxyPreference m_proxyPreference;
    QNetworkProxy::ProxyType m_proxyType;
    QString m_hostName;
    quint16 m_port;
    QString m_username;
    QString m_password;
    QStringList m_proxyExceptions;
};

#endif // NETWORKPROXYFACTORY_H
