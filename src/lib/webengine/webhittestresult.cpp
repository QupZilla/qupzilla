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
#include "webhittestresult.h"
#include "webpage.h"

WebHitTestResult::WebHitTestResult(const WebPage *page, const QPoint &pos)
    : m_isNull(true)
    , m_isContentEditable(false)
    , m_isContentSelected(false)
{
    QString source = QL1S("(function() {"
                          "var e = document.elementFromPoint(%1, %2);"
                          "console.log(e);"
                          "if (!e)"
                          "    return;"
                          "function isMediaElement(e) {"
                          "    return e.tagName == 'AUDIO' || e.tagName == 'VIDEO';"
                          "}"
                          "function isEditableElement(e) {"
                          "    if (e.isContentEditable)"
                          "        return true;"
                          "    if (e.tagName == 'INPUT' || e.tagName == 'TEXTAREA')"
                          "        return !e.readonly;"
                          "    return false;"
                          "}"
                          "var res = {"
                          "    alternateText: e.getAttribute('alt'),"
                          "    boundingRect: '',"
                          "    imageUrl: '',"
                          "    contentEditable: isEditableElement(e),"
                          "    contentSelected: window.getSelection().containsNode(e, true),"
                          "    linkTitle: '',"
                          "    linkUrl: '',"
                          "    mediaUrl: '',"
                          "    pos: '',"
                          "    tagName: e.tagName.toLowerCase()"
                          "};"
                          "var r = e.getBoundingClientRect();"
                          "res.boundingRect = [r.top, r.left, r.width, r.height];"
                          "res.pos = [r.top, r.left];"
                          "if (e.tagName == 'IMG')"
                          "    res.imageUrl = e.getAttribute('src');"
                          "if (e.tagName == 'A') {"
                          "    res.linkTitle = e.text;"
                          "    res.linkUrl = e.getAttribute('href');"
                          "}"
                          "if (isMediaElement(e))"
                          "    res.mediaUrl = e.getAttribute('src');"
                          "var pe = e.parentElement;"
                          "while (pe) {"
                          "    if (res.linkTitle == '' && pe.tagName == 'A')"
                          "        res.linkTitle = pe.text;"
                          "    if (res.linkUrl == '' && pe.tagName == 'A')"
                          "        res.linkUrl = pe.getAttribute('href');"
                          "    if (res.mediaUrl == '' && isMediaElement(pe))"
                          "        res.mediaUrl = pe.getAttribute('src');"
                          "    pe = pe.parentElement;"
                          "}"
                          "return res;"
                          "})()");

    WebPage *p = const_cast<WebPage*>(page);
    const QString &js = source.arg(QString::number(pos.x()), QString::number(pos.y()));
    init(page->url(), p->execJavaScript(js).toMap());
}

QString WebHitTestResult::alternateText() const
{
    return m_alternateText;
}

QRect WebHitTestResult::boundingRect() const
{
    return m_boundingRect;
}

QUrl WebHitTestResult::imageUrl() const
{
    return m_imageUrl;
}

bool WebHitTestResult::isContentEditable() const
{
    return m_isContentEditable;
}

bool WebHitTestResult::isContentSelected() const
{
    return m_isContentSelected;
}

bool WebHitTestResult::isNull() const
{
    return m_isNull;
}

QString WebHitTestResult::linkTitle() const
{
    return m_linkTitle;
}

QUrl WebHitTestResult::linkUrl() const
{
    return m_linkUrl;
}

QUrl WebHitTestResult::mediaUrl() const
{
    return m_mediaUrl;
}

QPoint WebHitTestResult::pos() const
{
    return m_pos;
}

QString WebHitTestResult::tagName() const
{
    return m_tagName;
}

void WebHitTestResult::init(const QUrl &url, const QVariantMap &map)
{
    if (map.isEmpty())
        return;

    m_alternateText = map.value(QSL("alternateText")).toString();
    m_imageUrl = map.value(QSL("imageUrl")).toUrl();
    m_isContentEditable = map.value(QSL("contentEditable")).toBool();
    m_isContentSelected = map.value(QSL("contentSelected")).toBool();
    m_linkTitle = map.value(QSL("linkTitle")).toString();
    m_linkUrl = map.value(QSL("linkUrl")).toUrl();
    m_mediaUrl = map.value(QSL("mediaUrl")).toUrl();
    m_tagName = map.value(QSL("tagName")).toString();

    const QVariantList &point = map.value(QSL("pos")).toList();
    if (point.size() == 2)
        m_pos = QPoint(point.at(0).toInt(), point.at(1).toInt());

    const QVariantList &rect = map.value(QSL("boundingRect")).toList();
    if (rect.size() == 4)
        m_boundingRect = QRect(rect.at(0).toInt(), rect.at(1).toInt(), rect.at(2).toInt(), rect.at(3).toInt());

    if (!m_linkUrl.isEmpty())
        m_linkUrl = url.resolved(m_linkUrl);
    if (!m_mediaUrl.isEmpty())
        m_mediaUrl = url.resolved(m_mediaUrl);
}

