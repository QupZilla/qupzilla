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
#include "adblockmanager.h"
#include "adblockdialog.h"
#include "adblockpage.h"
#include "adblocksubscription.h"
#include "adblockblockednetworkreply.h"
#include "mainapplication.h"
#include "webpage.h"
#include "globalfunctions.h"
#include "networkmanager.h"
#include "qupzilla.h"
#include "settings.h"

#include <QDateTime>
#include <QTextStream>
#include <QDir>
#include <QTimer>
#include <QDebug>

AdBlockManager* AdBlockManager::s_adBlockManager = 0;

AdBlockManager::AdBlockManager(QObject* parent)
    : QObject(parent)
    , m_loaded(false)
    , m_enabled(true)
{
}

AdBlockManager* AdBlockManager::instance()
{
    if (!s_adBlockManager) {
        s_adBlockManager = new AdBlockManager(mApp->networkManager());
    }

    return s_adBlockManager;
}

void AdBlockManager::setEnabled(bool enabled)
{
    if (isEnabled() == enabled) {
        return;
    }

    m_enabled = enabled;
    mApp->sendMessages(Qz::AM_SetAdBlockIconEnabled, enabled);
}

QList<AdBlockSubscription*> AdBlockManager::subscriptions() const
{
    return m_subscriptions;
}

QNetworkReply* AdBlockManager::block(const QNetworkRequest &request)
{
    const QString &urlString = request.url().toEncoded();
    const QString &urlScheme = request.url().scheme();

    if (!isEnabled() || urlScheme == "data" || urlScheme == "qrc" || urlScheme == "file" || urlScheme == "qupzilla") {
        return 0;
    }

    const AdBlockRule* blockedRule = 0;

    foreach(AdBlockSubscription * subscription, m_subscriptions) {
        if (subscription->allow(urlString)) {
            return 0;
        }

        if (const AdBlockRule* rule = subscription->block(urlString)) {
            blockedRule = rule;
        }

        if (blockedRule) {
            QVariant v = request.attribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 100));
            WebPage* webPage = static_cast<WebPage*>(v.value<void*>());
            if (WebPage::isPointerSafeToUse(webPage)) {
                webPage->addAdBlockRule(blockedRule->filter(), request.url());
            }

            AdBlockBlockedNetworkReply* reply = new AdBlockBlockedNetworkReply(subscription, blockedRule, this);
            reply->setRequest(request);

            return reply;
        }
    }

    return 0;
}

AdBlockSubscription* AdBlockManager::addSubscription(const QString &title, const QString &url)
{
    if (title.isEmpty() || url.isEmpty()) {
        return 0;
    }

    QString fileName = qz_filterCharsFromFilename(title.toLower()) + ".txt";
    QString filePath = qz_ensureUniqueFilename(mApp->currentProfilePath() + "adblock/" + fileName);

    QByteArray data = QString("Title: %1\nUrl: %2\n[Adblock Plus 1.1.1]").arg(title, url).toAscii();

    QFile file(filePath);
    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        qWarning() << "AdBlockManager: Cannot write to file" << filePath;
        return 0;
    }

    file.write(data);
    file.close();

    AdBlockSubscription* subscription = new AdBlockSubscription(title, this);
    subscription->setUrl(QUrl(url));
    subscription->setFilePath(filePath);
    subscription->loadSubscription();

    m_subscriptions.insert(m_subscriptions.count() - 1, subscription);

    return subscription;
}

bool AdBlockManager::removeSubscription(AdBlockSubscription* subscription)
{
    if (!m_subscriptions.contains(subscription) || !subscription->canBeRemoved()) {
        return false;
    }

    QFile(subscription->filePath()).remove();
    m_subscriptions.removeOne(subscription);

    delete subscription;
    return true;
}

void AdBlockManager::load()
{
    if (m_loaded) {
        return;
    }
    m_loaded = true;

    Settings settings;
    settings.beginGroup("AdBlock");
    m_enabled = settings.value("enabled", m_enabled).toBool();
    QDateTime lastUpdate = settings.value("lastUpdate", QDateTime()).toDateTime();
    settings.endGroup();

    QDir adblockDir(mApp->currentProfilePath() + "adblock");
    // Create if neccessary
    if (!adblockDir.exists()) {
        QDir(mApp->currentProfilePath()).mkdir("adblock");
    }

    foreach(const QString & fileName, adblockDir.entryList(QStringList("*.txt"), QDir::Files)) {
        if (fileName == "easylist.txt" || fileName == "customlist.txt") {
            continue;
        }

        const QString absolutePath = adblockDir.absoluteFilePath(fileName);
        QFile file(absolutePath);
        if (!file.open(QFile::ReadOnly)) {
            continue;
        }

        QTextStream textStream(&file);
        QString title = textStream.readLine(1024).remove("Title: ");
        QUrl url = QUrl(textStream.readLine(1024).remove("Url: "));

        if (title.isEmpty() || !url.isValid()) {
            qWarning() << "AdBlockManager: Invalid subscription file" << absolutePath;
            continue;
        }

        AdBlockSubscription* subscription = new AdBlockSubscription(title, this);
        subscription->setUrl(url);
        subscription->setFilePath(absolutePath);
        m_subscriptions.append(subscription);
    }

    // Prepend EasyList
    AdBlockSubscription* easyList = new AdBlockEasyList(this);
    m_subscriptions.prepend(easyList);

    // Append CustomList
    AdBlockSubscription* customList = new AdBlockCustomList(this);
    m_subscriptions.append(customList);

    // Load all subscriptions
    foreach(AdBlockSubscription * subscription, m_subscriptions) {
        subscription->loadSubscription();
    }

    if (lastUpdate.addDays(5) < QDateTime::currentDateTime()) {
        QTimer::singleShot(1000 * 60, this, SLOT(updateAllSubscriptions()));
    }
}

void AdBlockManager::updateAllSubscriptions()
{
    foreach(AdBlockSubscription * subscription, m_subscriptions) {
        subscription->updateSubscription();
    }

    Settings settings;
    settings.beginGroup("AdBlock");
    settings.setValue("lastUpdate", QDateTime::currentDateTime());
    settings.endGroup();
}

void AdBlockManager::save()
{
    if (!m_loaded) {
        return;
    }

    foreach(AdBlockSubscription * subscription, m_subscriptions) {
        subscription->saveSubscription();
    }

    Settings settings;
    settings.beginGroup("AdBlock");
    settings.setValue("enabled", m_enabled);
    settings.endGroup();
}

AdBlockDialog* AdBlockManager::showDialog()
{
    if (!m_adBlockDialog) {
        m_adBlockDialog = new AdBlockDialog(mApp->getWindow());
    }

    m_adBlockDialog.data()->show();
    return m_adBlockDialog.data();
}

void AdBlockManager::showRule()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        showDialog()->search->setText(action->data().toString());
    }
}
