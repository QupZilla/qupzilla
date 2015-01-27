/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2013-2014  David Rosca <nowrep@gmail.com>
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
#include "html5permissionsmanager.h"
#include "html5permissionsnotification.h"
#include "settings.h"
#include "webview.h"


HTML5PermissionsManager::HTML5PermissionsManager(QObject* parent)
    : QObject(parent)
{
    loadSettings();
}

#if QTWEBENGINE_DISABLED
void HTML5PermissionsManager::requestPermissions(WebPage* page, QWebEngineFrame* frame, const QWebEnginePage::Feature &feature)
{
    if (!frame || !page) {
        return;
    }

    const QString host = page->url().host();
    WebView* view = qobject_cast<WebView*>(page->view());

    switch (feature) {
    case QWebEnginePage::Notifications:
        if (m_notificationsGranted.contains(host)) {
            page->setFeaturePermission(frame, feature, QWebEnginePage::PermissionGrantedByUser);
            return;
        }

        if (m_notificationsDenied.contains(host)) {
            page->setFeaturePermission(frame, feature, QWebEnginePage::PermissionDeniedByUser);
            return;
        }

        if (view) {
            HTML5PermissionsNotification* notif = new HTML5PermissionsNotification(host, frame, feature);
            view->addNotification(notif);
        }

        break;

    case QWebEnginePage::Geolocation:
        if (m_geolocationGranted.contains(host)) {
            page->setFeaturePermission(frame, feature, QWebEnginePage::PermissionGrantedByUser);
            return;
        }

        if (m_geolocationDenied.contains(host)) {
            page->setFeaturePermission(frame, feature, QWebEnginePage::PermissionDeniedByUser);
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

void HTML5PermissionsManager::rememberPermissions(const QString &host, const QWebEnginePage::Feature &feature,
        const QWebEnginePage::PermissionPolicy &policy)
{
    if (host.isEmpty()) {
        return;
    }

    switch (feature) {
    case QWebEnginePage::Notifications:
        if (policy == QWebEnginePage::PermissionGrantedByUser) {
            m_notificationsGranted.append(host);
        }
        else {
            m_notificationsDenied.append(host);
        }
        break;

    case QWebEnginePage::Geolocation:
        if (policy == QWebEnginePage::PermissionGrantedByUser) {
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

