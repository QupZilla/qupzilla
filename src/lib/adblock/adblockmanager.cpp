/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
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
#include "adblocksubscription.h"
#include "adblockblockednetworkreply.h"
#include "datapaths.h"
#include "mainapplication.h"
#include "webpage.h"
#include "qztools.h"
#include "networkmanager.h"
#include "browserwindow.h"
#include "settings.h"

#include <QDateTime>
#include <QTextStream>
#include <QDir>
#include <QTimer>

//#define ADBLOCK_DEBUG

#ifdef ADBLOCK_DEBUG
#include <QElapsedTimer>
#endif

Q_GLOBAL_STATIC(AdBlockManager, qz_adblock_manager)

AdBlockManager::AdBlockManager(QObject* parent)
    : QObject(parent)
    , m_loaded(false)
    , m_enabled(true)
    , m_useLimitedEasyList(true)
{
    load();
}

AdBlockManager::~AdBlockManager()
{
    qDeleteAll(m_subscriptions);
}

AdBlockManager* AdBlockManager::instance()
{
    return qz_adblock_manager();
}

void AdBlockManager::setEnabled(bool enabled)
{
    if (m_enabled == enabled) {
        return;
    }

    m_enabled = enabled;
    emit enabledChanged(enabled);

    Settings settings;
    settings.beginGroup("AdBlock");
    settings.setValue("enabled", m_enabled);
    settings.endGroup();

    load();
    mApp->reloadUserStyleSheet();
}

QList<AdBlockSubscription*> AdBlockManager::subscriptions() const
{
    return m_subscriptions;
}

QNetworkReply* AdBlockManager::block(const QNetworkRequest &request)
{
#ifdef ADBLOCK_DEBUG
    QElapsedTimer timer;
    timer.start();
#endif
    const QString urlString = request.url().toEncoded().toLower();
    const QString urlDomain = request.url().host().toLower();
    const QString urlScheme = request.url().scheme().toLower();

    if (!isEnabled() || !canRunOnScheme(urlScheme)) {
        return 0;
    }

    foreach (AdBlockSubscription* subscription, m_subscriptions) {
        const AdBlockRule* blockedRule = subscription->match(request, urlDomain, urlString);

        if (blockedRule) {
            QVariant v = request.attribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 100));
            WebPage* webPage = static_cast<WebPage*>(v.value<void*>());
            if (WebPage::isPointerSafeToUse(webPage)) {
                if (!canBeBlocked(webPage->url())) {
                    return 0;
                }

                webPage->addAdBlockRule(blockedRule, request.url());
            }

            AdBlockBlockedNetworkReply* reply = new AdBlockBlockedNetworkReply(subscription, blockedRule, this);
            reply->setRequest(request);

#ifdef ADBLOCK_DEBUG
            qDebug() << "BLOCKED: " << timer.elapsed() << blockedRule->filter() << request.url();
#endif

            return reply;
        }
    }

#ifdef ADBLOCK_DEBUG
    qDebug() << timer.elapsed() << request.url();
#endif

    return 0;
}

QStringList AdBlockManager::disabledRules() const
{
    return m_disabledRules;
}

void AdBlockManager::addDisabledRule(const QString &filter)
{
    m_disabledRules.append(filter);
}

void AdBlockManager::removeDisabledRule(const QString &filter)
{
    m_disabledRules.removeOne(filter);
}

AdBlockSubscription* AdBlockManager::addSubscription(const QString &title, const QString &url)
{
    if (title.isEmpty() || url.isEmpty()) {
        return 0;
    }

    QString fileName = QzTools::filterCharsFromFilename(title.toLower()) + ".txt";
    QString filePath = QzTools::ensureUniqueFilename(DataPaths::currentProfilePath() + "/adblock/" + fileName);

    QByteArray data = QString("Title: %1\nUrl: %2\n[Adblock Plus 1.1.1]").arg(title, url).toLatin1();

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
    subscription->loadSubscription(m_disabledRules);

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

AdBlockCustomList* AdBlockManager::customList() const
{
    foreach (AdBlockSubscription* subscription, m_subscriptions) {
        AdBlockCustomList* list = qobject_cast<AdBlockCustomList*>(subscription);

        if (list) {
            return list;
        }
    }

    return 0;
}

void AdBlockManager::load()
{
    if (m_loaded) {
        return;
    }

#ifdef ADBLOCK_DEBUG
    QElapsedTimer timer;
    timer.start();
#endif

    Settings settings;
    settings.beginGroup("AdBlock");
    m_enabled = settings.value("enabled", m_enabled).toBool();
    m_useLimitedEasyList = settings.value("useLimitedEasyList", m_useLimitedEasyList).toBool();
    m_disabledRules = settings.value("disabledRules", QStringList()).toStringList();
    QDateTime lastUpdate = settings.value("lastUpdate", QDateTime()).toDateTime();
    settings.endGroup();

    if (!m_enabled) {
        return;
    }

    QDir adblockDir(DataPaths::currentProfilePath() + "/adblock");
    // Create if neccessary
    if (!adblockDir.exists()) {
        QDir(DataPaths::currentProfilePath()).mkdir("adblock");
    }

    foreach (const QString &fileName, adblockDir.entryList(QStringList("*.txt"), QDir::Files)) {
        if (fileName == QLatin1String("customlist.txt")) {
            continue;
        }

        const QString absolutePath = adblockDir.absoluteFilePath(fileName);
        QFile file(absolutePath);
        if (!file.open(QFile::ReadOnly)) {
            continue;
        }

        QTextStream textStream(&file);
        textStream.setCodec("UTF-8");
        QString title = textStream.readLine(1024).remove(QLatin1String("Title: "));
        QUrl url = QUrl(textStream.readLine(1024).remove(QLatin1String("Url: ")));

        if (title.isEmpty() || !url.isValid()) {
            qWarning() << "AdBlockManager: Invalid subscription file" << absolutePath;
            continue;
        }

        AdBlockSubscription* subscription = new AdBlockSubscription(title, this);
        subscription->setUrl(url);
        subscription->setFilePath(absolutePath);
        connect(subscription, SIGNAL(subscriptionUpdated()), mApp, SLOT(reloadUserStyleSheet()));

        m_subscriptions.append(subscription);
    }

    // Prepend EasyList if subscriptions are empty
    if (m_subscriptions.isEmpty()) {
        AdBlockSubscription* easyList = new AdBlockSubscription(tr("EasyList"), this);
        easyList->setUrl(QUrl(ADBLOCK_EASYLIST_URL));
        easyList->setFilePath(DataPaths::currentProfilePath() + QLatin1String("/adblock/easylist.txt"));
        connect(easyList, SIGNAL(subscriptionUpdated()), mApp, SLOT(reloadUserStyleSheet()));

        m_subscriptions.prepend(easyList);
    }

    // Append CustomList
    AdBlockCustomList* customList = new AdBlockCustomList(this);
    m_subscriptions.append(customList);
    connect(customList, SIGNAL(subscriptionEdited()), mApp, SLOT(reloadUserStyleSheet()));

    // Load all subscriptions
    foreach (AdBlockSubscription* subscription, m_subscriptions) {
        subscription->loadSubscription(m_disabledRules);
    }

    if (lastUpdate.addDays(5) < QDateTime::currentDateTime()) {
        QTimer::singleShot(1000 * 60, this, SLOT(updateAllSubscriptions()));
    }

#ifdef ADBLOCK_DEBUG
    qDebug() << "AdBlock loaded in" << timer.elapsed();
#endif

    m_loaded = true;
}

void AdBlockManager::updateAllSubscriptions()
{
    foreach (AdBlockSubscription* subscription, m_subscriptions) {
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

    foreach (AdBlockSubscription* subscription, m_subscriptions) {
        subscription->saveSubscription();
    }

    Settings settings;
    settings.beginGroup("AdBlock");
    settings.setValue("enabled", m_enabled);
    settings.setValue("useLimitedEasyList", m_useLimitedEasyList);
    settings.setValue("disabledRules", m_disabledRules);
    settings.endGroup();
}

bool AdBlockManager::isEnabled() const
{
    return m_enabled;
}

bool AdBlockManager::canRunOnScheme(const QString &scheme) const
{
    return !(scheme == QLatin1String("file") || scheme == QLatin1String("qrc")
             || scheme == QLatin1String("qupzilla") || scheme == QLatin1String("data")
             || scheme == QLatin1String("abp"));
}

bool AdBlockManager::useLimitedEasyList() const
{
    return m_useLimitedEasyList;
}

void AdBlockManager::setUseLimitedEasyList(bool useLimited)
{
    m_useLimitedEasyList = useLimited;

    foreach (AdBlockSubscription* subscription, m_subscriptions) {
        if (subscription->url() == QUrl(ADBLOCK_EASYLIST_URL)) {
            subscription->updateSubscription();
        }
    }
}

bool AdBlockManager::canBeBlocked(const QUrl &url) const
{
    foreach (AdBlockSubscription* subscription, m_subscriptions) {
        if (subscription->adBlockDisabledForUrl(url)) {
            return false;
        }
    }

    return true;
}

QString AdBlockManager::elementHidingRules() const
{
    if (!m_enabled) {
        return QString();
    }

    QString rules;

    foreach (AdBlockSubscription* subscription, m_subscriptions) {
        rules.append(subscription->elementHidingRules());
    }

    return rules;
}

QString AdBlockManager::elementHidingRulesForDomain(const QUrl &url) const
{
    if (!isEnabled() || !canRunOnScheme(url.scheme()) || !canBeBlocked(url)) {
        return QString();
    }

    // Acid3 doesn't like the way element hiding rules are embedded into page
    if (url.host() == QLatin1String("acid3.acidtests.org")) {
        return QString();
    }

    QString rules;

    foreach (AdBlockSubscription* subscription, m_subscriptions) {
        if (subscription->elemHideDisabledForUrl(url)) {
            return QString();
        }

        rules.append(subscription->elementHidingRulesForDomain(url.host()));
    }

    // Remove last ","
    if (!rules.isEmpty()) {
        rules = rules.left(rules.size() - 1);
    }

    return rules;
}

AdBlockSubscription* AdBlockManager::subscriptionByName(const QString &name) const
{
    foreach (AdBlockSubscription* subscription, m_subscriptions) {
        if (subscription->title() == name) {
            return subscription;
        }
    }

    return 0;
}

AdBlockDialog* AdBlockManager::showDialog()
{
    if (!m_adBlockDialog) {
        m_adBlockDialog = new AdBlockDialog;
    }

    m_adBlockDialog.data()->show();
    m_adBlockDialog.data()->raise();
    m_adBlockDialog.data()->activateWindow();

    return m_adBlockDialog.data();
}

void AdBlockManager::showRule()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        const AdBlockRule* rule = static_cast<const AdBlockRule*>(action->data().value<void*>());

        if (rule) {
            showDialog()->showRule(rule);
        }
    }
}
