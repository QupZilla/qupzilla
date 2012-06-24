/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
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
#ifndef ADBLOCKMANAGER_H
#define ADBLOCKMANAGER_H

#include <QObject>
#include <QWeakPointer>

#include "qz_namespace.h"

class QUrl;
class QNetworkReply;
class QNetworkRequest;

class AdBlockDialog;
class AdBlockSubscription;

class QT_QUPZILLA_EXPORT AdBlockManager : public QObject
{
    Q_OBJECT

public:
    AdBlockManager(QObject* parent = 0);
    static AdBlockManager* instance();

    void load();
    void save();

    bool isEnabled() { if (!m_loaded) load(); return m_enabled; }

    QList<AdBlockSubscription*> subscriptions() const;
    QNetworkReply* block(const QNetworkRequest &request);

    AdBlockSubscription* addSubscription(const QString &title, const QString &url);
    bool removeSubscription(AdBlockSubscription* subscription);

public slots:
    void setEnabled(bool enabled);
    void showRule();

    void updateAllSubscriptions();

    AdBlockDialog* showDialog();

private:
    static AdBlockManager* s_adBlockManager;

    bool m_loaded;
    bool m_enabled;

    QWeakPointer<AdBlockDialog> m_adBlockDialog;
    QList<AdBlockSubscription*> m_subscriptions;
};

#endif // ADBLOCKMANAGER_H

