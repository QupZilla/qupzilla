/* ============================================================
* QupZilla - QtWebEngine based browser
* Copyright (C) 2015 David Rosca <nowrep@gmail.com>
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
#ifndef WEBHITTESTRESULT_H
#define WEBHITTESTRESULT_H

#include <QUrl>
#include <QRect>
#include <QString>
#include <QVariantMap>

class WebPage;

class WebHitTestResult
{
public:
    explicit WebHitTestResult(const WebPage *page, const QPoint &pos);

    QString alternateText() const;
    QRect boundingRect() const;
    QUrl imageUrl() const;
    bool isContentEditable() const;
    bool isContentSelected() const;
    bool isNull() const;
    QString linkTitle() const;
    QUrl linkUrl() const;
    QUrl mediaUrl() const;
    QPoint pos() const;
    QString tagName() const;

private:
    void init(const QUrl &url, const QVariantMap &map);

    bool m_isNull;
    QString m_alternateText;
    QRect m_boundingRect;
    QUrl m_imageUrl;
    bool m_isContentEditable;
    bool m_isContentSelected;
    QString m_linkTitle;
    QUrl m_linkUrl;
    QUrl m_mediaUrl;
    QPoint m_pos;
    QString m_tagName;
};

#endif // WEBHITTESTRESULT_H
