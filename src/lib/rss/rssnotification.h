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
#ifndef RSSNOTIFICATION_H
#define RSSNOTIFICATION_H

#include <QUrl>
#include <QIcon>

#include "qz_namespace.h"
#include "animatedwidget.h"

class WebView;

namespace Ui
{
class RSSNotification;
}

class QT_QUPZILLA_EXPORT RSSNotification : public AnimatedWidget
{
    Q_OBJECT
public:
    explicit RSSNotification(const QString &title, const QUrl &url, WebView* parent = 0);
    ~RSSNotification();

public slots:
    void hide();

private slots:
    void addRss();

private:
    enum AppType { WebApplication, DesktopApplication, Internal, Other };
    struct RssApp {
        QString title;
        QString address;
        QIcon icon;
        AppType type;

        RssApp(const QString &t, const QString &a, const QIcon &i, AppType ty = WebApplication) {
            title = t;
            address = a;
            icon = i;
            type = ty;
        }
    };

    Ui::RSSNotification* ui;

    QString m_title;
    QUrl m_url;
    WebView* m_view;

    QList<RssApp> m_rssApps;
};

#endif // RSSNOTIFICATION_H
