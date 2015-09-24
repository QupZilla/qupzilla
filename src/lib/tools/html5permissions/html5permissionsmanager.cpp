/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2013-2015  David Rosca <nowrep@gmail.com>
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

void HTML5PermissionsManager::requestPermissions(WebPage* page, const QUrl &origin, const QWebEnginePage::Feature &feature)
{
    if (!page) {
        return;
    }

    if (!m_granted.contains(feature) || !m_denied.contains(feature)) {
        qWarning() << "HTML5PermissionsManager: Unknown feature" << feature;
        return;
    }

    // Permission granted
    if (m_granted.value(feature).contains(origin.toString())) {
        page->setFeaturePermission(origin, feature, QWebEnginePage::PermissionGrantedByUser);
        return;
    }

    // Permission denied
    if (m_denied.value(feature).contains(origin.toString())) {
        page->setFeaturePermission(origin, feature, QWebEnginePage::PermissionDeniedByUser);
        return;
    }

    // Ask user for permission
    HTML5PermissionsNotification* notif = new HTML5PermissionsNotification(origin, page, feature);
    page->view()->addNotification(notif);
}

void HTML5PermissionsManager::rememberPermissions(const QUrl &origin, const QWebEnginePage::Feature &feature,
        const QWebEnginePage::PermissionPolicy &policy)
{
    if (origin.isEmpty()) {
        return;
    }

    if (policy == QWebEnginePage::PermissionGrantedByUser) {
        m_granted[feature].append(origin.toString());
    }
    else {
        m_denied[feature].append(origin.toString());
    }

    saveSettings();
}

void HTML5PermissionsManager::loadSettings()
{
    Settings settings;
    settings.beginGroup("HTML5Notifications");

    m_granted[QWebEnginePage::Notifications] = settings.value("NotificationsGranted", QStringList()).toStringList();
    m_denied[QWebEnginePage::Notifications] = settings.value("NotificationsDenied", QStringList()).toStringList();

    m_granted[QWebEnginePage::Geolocation] = settings.value("GeolocationGranted", QStringList()).toStringList();
    m_denied[QWebEnginePage::Geolocation] = settings.value("GeolocationDenied", QStringList()).toStringList();

    m_granted[QWebEnginePage::MediaAudioCapture] = settings.value("MediaAudioCaptureGranted", QStringList()).toStringList();
    m_denied[QWebEnginePage::MediaAudioCapture] = settings.value("MediaAudioCaptureDenied", QStringList()).toStringList();

    m_granted[QWebEnginePage::MediaVideoCapture] = settings.value("MediaVideoCaptureGranted", QStringList()).toStringList();
    m_denied[QWebEnginePage::MediaVideoCapture] = settings.value("MediaVideoCaptureDenied", QStringList()).toStringList();

    m_granted[QWebEnginePage::MediaAudioVideoCapture] = settings.value("MediaAudioVideoCaptureGranted", QStringList()).toStringList();
    m_denied[QWebEnginePage::MediaAudioVideoCapture] = settings.value("MediaAudioVideoCaptureDenied", QStringList()).toStringList();

    m_granted[QWebEnginePage::MouseLock] = settings.value("MouseLockGranted", QStringList()).toStringList();
    m_denied[QWebEnginePage::MouseLock] = settings.value("MouseLockDenied", QStringList()).toStringList();

    settings.endGroup();
}

void HTML5PermissionsManager::saveSettings()
{
    Settings settings;
    settings.beginGroup("HTML5Notifications");

    settings.setValue("NotificationsGranted", m_granted[QWebEnginePage::Notifications]);
    settings.setValue("NotificationsDenied", m_denied[QWebEnginePage::Notifications]);

    settings.setValue("GeolocationGranted", m_granted[QWebEnginePage::Geolocation]);
    settings.setValue("GeolocationDenied", m_denied[QWebEnginePage::Geolocation]);

    settings.setValue("MediaAudioCaptureGranted", m_granted[QWebEnginePage::MediaAudioCapture]);
    settings.setValue("MediaAudioCaptureDenied", m_denied[QWebEnginePage::MediaAudioCapture]);

    settings.setValue("MediaVideoCaptureGranted", m_granted[QWebEnginePage::MediaVideoCapture]);
    settings.setValue("MediaVideoCaptureDenied", m_denied[QWebEnginePage::MediaVideoCapture]);

    settings.setValue("MediaAudioVideoCaptureGranted", m_granted[QWebEnginePage::MediaAudioVideoCapture]);
    settings.setValue("MediaAudioVideoCaptureDenied", m_denied[QWebEnginePage::MediaAudioVideoCapture]);

    settings.setValue("MouseLockGranted", m_granted[QWebEnginePage::MouseLock]);
    settings.setValue("MouseLockDenied", m_denied[QWebEnginePage::MouseLock]);

    settings.endGroup();
}
