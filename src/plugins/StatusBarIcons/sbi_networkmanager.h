/* ============================================================
* StatusBarIcons - Extra icons in statusbar for QupZilla
* Copyright (C) 2013  David Rosca <nowrep@gmail.com>
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
#ifndef SBI_NETWORKMANAGER_H
#define SBI_NETWORKMANAGER_H

#include <QObject>
#include <QHash>

class SBI_NetworkProxy;

class SBI_NetworkManager : public QObject
{
    Q_OBJECT
public:
    explicit SBI_NetworkManager(const QString &settingsPath, QObject* parent = 0);
    ~SBI_NetworkManager();

    static SBI_NetworkManager* instance();

    void loadSettings();

    QString currentProxyName() const;
    SBI_NetworkProxy* currentProxy() const;
    void setCurrentProxy(const QString &name);

    void saveProxy(const QString &name, SBI_NetworkProxy* proxy);
    void removeProxy(const QString &name);

    QHash<QString, SBI_NetworkProxy*> proxies() const;

private:
    void applyCurrentProxy();
    void deleteProxies();

    QString m_settingsFile;
    QHash<QString, SBI_NetworkProxy*> m_proxies;
    SBI_NetworkProxy* m_currentProxy;

    static SBI_NetworkManager* s_instance;
};

#define SBINetManager SBI_NetworkManager::instance()

#endif // SBI_NETWORKMANAGER_H
