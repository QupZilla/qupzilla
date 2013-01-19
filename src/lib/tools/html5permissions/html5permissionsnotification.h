#ifndef HTML5PERMISSIONSNOTIFICATION_H
#define HTML5PERMISSIONSNOTIFICATION_H

#include <QWebPage>
#include <QString>

#include "animatedwidget.h"

namespace Ui
{
class HTML5PermissionsNotification;
}

class HTML5PermissionsNotification : public AnimatedWidget
{
    Q_OBJECT

public:
    explicit HTML5PermissionsNotification(const QString &host, QWebFrame* frame, const QWebPage::Feature &feature);
    ~HTML5PermissionsNotification();

private slots:
    void grantPermissions();
    void denyPermissions();

private:
    Ui::HTML5PermissionsNotification* ui;

    QString m_host;
    QWebFrame* m_frame;
    QWebPage::Feature m_feature;
};

#endif // HTML5PERMISSIONSNOTIFICATION_H
