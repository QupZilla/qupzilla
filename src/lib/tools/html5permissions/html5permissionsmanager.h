#ifndef HTML5PERMISSIONSMANAGER_H
#define HTML5PERMISSIONSMANAGER_H

#include <QObject>
#include <QStringList>
#include <QWebPage>

#include "qz_namespace.h"

class QUrl;
class WebPage;

class QT_QUPZILLA_EXPORT HTML5PermissionsManager : public QObject
{
public:
    explicit HTML5PermissionsManager(QObject* parent);

    void requestPermissions(WebPage* page, QWebFrame* frame, const QWebPage::Feature &feature);
    void rememberPermissions(const QString &host, const QWebPage::Feature &feature,
                             const QWebPage::PermissionPolicy &policy);

    void loadSettings();
    void showSettingsDialog();

private:
    void saveSettings();

    QStringList m_notificationsGranted;
    QStringList m_notificationsDenied;

    QStringList m_geolocationGranted;
    QStringList m_geolocationDenied;
};

#endif // HTML5PERMISSIONSMANAGER_H
