/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2018 David Rosca <nowrep@gmail.com>
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
#pragma once

#include <QIcon>
#include <QObject>

#include <functional>

#include "qzcommon.h"

class WebView;

class QUPZILLA_EXPORT AbstractButtonInterface : public QObject
{
    Q_OBJECT

public:
    struct ClickController {
        QWidget *visualParent;
        std::function<QPoint(QSize)> popupPosition;
        bool popupOpened = false;
        std::function<void()> popupClosed;
    };

    explicit AbstractButtonInterface(QObject *parent = nullptr);

    virtual QString id() const = 0;
    virtual QString name() const = 0;

    bool isValid() const;

    bool isActive() const;
    void setActive(bool active);

    bool isVisible() const;
    void setVisible(bool visible);

    QString title() const;
    void setTitle(const QString &text);

    QString toolTip() const;
    void setToolTip(const QString &toolTip);

    QIcon icon() const;
    void setIcon(const QIcon &icon);

    QString badgeText() const;
    void setBadgeText(const QString &badgeText);

    WebView *webView() const;
    void setWebView(WebView *view);

signals:
    void activeChanged(bool active);
    void visibleChanged(bool visible);
    void titleChanged(const QString &title);
    void toolTipChanged(const QString &toolTip);
    void iconChanged(const QIcon &icon);
    void badgeTextChanged(const QString &badgeText);
    void webViewChanged(WebView *view);
    void clicked(ClickController *controller);

private:
    bool m_active = true;
    bool m_visible = true;
    QString m_title;
    QString m_toolTip;
    QIcon m_icon;
    QString m_badgeText;
    WebView *m_view = nullptr;
};
