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
#ifndef RSSNOTIFICATION_H
#define RSSNOTIFICATION_H

#include <QUrl>
#include <QIcon>
#include <QVector>

#include "qzcommon.h"
#include "animatedwidget.h"

#if QTWEBENGINE_DISABLED

class WebView;

namespace Ui
{
class RSSNotification;
}

class QUPZILLA_EXPORT RSSNotification : public AnimatedWidget
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
        AppType type;
        QString title;
        QIcon icon;
        QString address;
        QString executable;
        QString arguments;
    };

    Ui::RSSNotification* ui;

    QString m_title;
    QUrl m_url;
    WebView* m_view;

    QVector<RssApp> m_rssApps;
};

#endif

#endif // RSSNOTIFICATION_H
