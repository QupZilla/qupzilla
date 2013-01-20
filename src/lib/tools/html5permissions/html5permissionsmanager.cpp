#include "html5permissionsmanager.h"
#include "html5permissionsnotification.h"
#include "settings.h"
#include "webview.h"

#include <QWebFrame>
#include <QDebug>

HTML5PermissionsManager::HTML5PermissionsManager(QObject *parent)
    : QObject(parent)
{
    loadSettings();
}

#if QTWEBKIT_FROM_2_2
void HTML5PermissionsManager::requestPermissions(WebPage* page, QWebFrame *frame, const QWebPage::Feature &feature)
{
    if (!frame || !page) {
        return;
    }

    const QString &host = page->url().host();
    WebView* view = qobject_cast<WebView*>(page->view());

    switch (feature) {
    case QWebPage::Notifications:
        if (m_notificationsGranted.contains(host)) {
            page->setFeaturePermission(frame, feature, QWebPage::PermissionGrantedByUser);
            return;
        }

        if (m_notificationsDenied.contains(host)) {
            page->setFeaturePermission(frame, feature, QWebPage::PermissionDeniedByUser);
            return;
        }

        if (view) {
            HTML5PermissionsNotification* notif = new HTML5PermissionsNotification(host, frame, feature);
            view->addNotification(notif);
        }

        break;

    case QWebPage::Geolocation:
        if (m_geolocationGranted.contains(host)) {
            page->setFeaturePermission(frame, feature, QWebPage::PermissionGrantedByUser);
            return;
        }

        if (m_geolocationDenied.contains(host)) {
            page->setFeaturePermission(frame, feature, QWebPage::PermissionDeniedByUser);
            return;
        }

        if (view) {
            HTML5PermissionsNotification* notif = new HTML5PermissionsNotification(host, frame, feature);
            view->addNotification(notif);
        }

        break;

    default:
        qWarning() << "HTML5PermissionsManager: Unknown feature" << feature;
        break;
    }
}

void HTML5PermissionsManager::rememberPermissions(const QString &host, const QWebPage::Feature &feature,
                                                  const QWebPage::PermissionPolicy &policy)
{
    if (host.isEmpty()) {
        return;
    }

    switch (feature) {
    case QWebPage::Notifications:
        if (policy == QWebPage::PermissionGrantedByUser) {
            m_notificationsGranted.append(host);
        }
        else {
            m_notificationsDenied.append(host);
        }
        break;

    case QWebPage::Geolocation:
        if (policy == QWebPage::PermissionGrantedByUser) {
            m_geolocationGranted.append(host);
        }
        else {
            m_geolocationDenied.append(host);
        }
        break;

    default:
        qWarning() << "HTML5PermissionsManager: Unknown feature" << feature;
        break;
    }

    saveSettings();
}
#endif

void HTML5PermissionsManager::loadSettings()
{
    Settings settings;
    settings.beginGroup("HTML5Notifications");
    m_notificationsGranted = settings.value("NotificationsGranted", QStringList()).toStringList();
    m_notificationsDenied = settings.value("NotificationsDenied", QStringList()).toStringList();
    m_geolocationGranted = settings.value("GeolocationGranted", QStringList()).toStringList();
    m_geolocationDenied = settings.value("GeolocationDenied", QStringList()).toStringList();
    settings.endGroup();
}

void HTML5PermissionsManager::saveSettings()
{
    Settings settings;
    settings.beginGroup("HTML5Notifications");
    settings.setValue("NotificationsGranted", m_notificationsGranted);
    settings.setValue("NotificationsDenied", m_notificationsDenied);
    settings.setValue("GeolocationGranted", m_geolocationGranted);
    settings.setValue("GeolocationDenied", m_geolocationDenied);
    settings.endGroup();
}

void HTML5PermissionsManager::showSettingsDialog()
{
}
