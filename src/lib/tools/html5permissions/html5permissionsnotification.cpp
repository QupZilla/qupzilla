#include "ui_html5permissionsnotification.h"
#include "html5permissionsnotification.h"
#include "html5permissionsmanager.h"
#include "mainapplication.h"
#include "iconprovider.h"

#include <QWebFrame>

#if QTWEBKIT_FROM_2_2
HTML5PermissionsNotification::HTML5PermissionsNotification(const QString &host, QWebFrame* frame, const QWebPage::Feature &feature)
    : AnimatedWidget(AnimatedWidget::Down, 300, 0)
    , ui(new Ui::HTML5PermissionsNotification)
    , m_host(host)
    , m_frame(frame)
    , m_feature(feature)
{
    ui->setupUi(widget());

    ui->close->setIcon(qIconProvider->standardIcon(QStyle::SP_DialogCloseButton));

    QString message;
    QString site = m_host.isEmpty() ? tr("this site") : QString("<b>%1</b>").arg(m_host);

    if (feature == QWebPage::Notifications) {
        ui->iconLabel->setPixmap(QPixmap(":icons/other/notification.png"));
        message = tr("Allow %1 to show desktop notifications?").arg(site);
    }
    else if (feature == QWebPage::Geolocation) {
        ui->iconLabel->setPixmap(QPixmap(":icons/other/geolocation.png"));
        message = tr("Allow %1 to locate your position?").arg(site);
    }

    ui->textLabel->setText(message);

    connect(ui->allow, SIGNAL(clicked()), this, SLOT(grantPermissions()));
    connect(ui->deny, SIGNAL(clicked()), this, SLOT(denyPermissions()));
    connect(ui->close, SIGNAL(clicked()), this, SLOT(denyPermissions()));

    startAnimation();
}

void HTML5PermissionsNotification::grantPermissions()
{
    if (!m_frame || !m_frame->page()) {
        return;
    }

    QWebPage* page = m_frame->page();
    page->setFeaturePermission(m_frame, m_feature, QWebPage::PermissionGrantedByUser);

    if (ui->remember->isChecked()) {
        mApp->html5permissions()->rememberPermissions(m_host, m_feature, QWebPage::PermissionGrantedByUser);
    }

    hide();
}

void HTML5PermissionsNotification::denyPermissions()
{
    if (!m_frame || !m_frame->page()) {
        return;
    }

    QWebPage* page = m_frame->page();
    page->setFeaturePermission(m_frame, m_feature, QWebPage::PermissionDeniedByUser);

    if (ui->remember->isChecked()) {
        mApp->html5permissions()->rememberPermissions(m_host, m_feature, QWebPage::PermissionDeniedByUser);
    }

    hide();
}

HTML5PermissionsNotification::~HTML5PermissionsNotification()
{
    delete ui;
}
#endif // QTWEBKIT_FROM_2_2
