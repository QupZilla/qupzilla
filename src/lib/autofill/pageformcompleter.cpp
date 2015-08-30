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
#include "pageformcompleter.h"
#include "qzregexp.h"

#include <QWebEnginePage>
#include <QUrlQuery>

PageFormCompleter::PageFormCompleter()
    : m_page(0)
    , m_frame(0)
{
}

PageFormData PageFormCompleter::extractFormData(QWebEnginePage* page, const QByteArray &postData)
{
    m_page = page;
    return extractFormData(postData);
}

PageFormData PageFormCompleter::extractFormData(QWebEngineFrame* frame, const QByteArray &postData)
{
    m_frame = frame;
    return extractFormData(postData);
}

bool PageFormCompleter::completeFormData(QWebEnginePage* page, const QByteArray &data)
{
    m_page = page;
    return completeFormData(data);
}

bool PageFormCompleter::completeFormData(QWebEngineFrame* frame, const QByteArray &data)
{
    m_frame = frame;
    return completeFormData(data);
}

PageFormData PageFormCompleter::extractFormData(const QByteArray &postData) const
{
    QString usernameValue;
    QString passwordValue;

    QByteArray data = convertWebKitFormBoundaryIfNecessary(postData);
    PageFormData formData = {QString(), QString(), data};

    if (data.isEmpty() || !data.contains('=')) {
        return formData;
    }

    const QueryItems queryItems = createQueryItems(data);

    if (queryItems.isEmpty()) {
        return formData;
    }

#if QTWEBENGINE_DISABLED
    const QWebElementCollection allForms = getAllElementsFromPage("form");

    // Find form that contains password value sent in data
    foreach (const QWebElement &formElement, allForms) {
        bool found = false;
        const QWebElementCollection inputs = formElement.findAll("input[type=\"password\"]");

        foreach (QWebElement inputElement, inputs) {
            const QString passName = inputElement.attribute("name");
            const QString passValue = inputElement.evaluateJavaScript("this.value").toString();

            if (queryItemsContains(queryItems, passName, passValue)) {
                // Set passwordValue if not empty (to make it possible extract forms without username field)
                passwordValue = passValue;

                const QueryItem item = findUsername(formElement);
                if (queryItemsContains(queryItems, item.first, item.second)) {
                    usernameValue = item.second;
                    found = true;
                    break;
                }
            }
        }

        if (found) {
            break;
        }
    }

    // It is necessary only to find password, as there may be form without username field
    if (passwordValue.isEmpty()) {
        return formData;
    }

    formData.username = usernameValue;
    formData.password = passwordValue;
#endif

    return formData;
}

// Returns if any data was actually filled in page
bool PageFormCompleter::completeFormData(const QByteArray &data) const
{
    bool completed = false;
    const QueryItems queryItems = createQueryItems(data);

    // Input types that are being completed
    QStringList inputTypes;
    inputTypes << "text" << "password" << "email";

#if QTWEBENGINE_DISABLED
    // Find all input elements in the page
    const QWebElementCollection inputs = getAllElementsFromPage("input");

    for (int i = 0; i < queryItems.count(); i++) {
        const QString key = queryItems.at(i).first;
        const QString value = queryItems.at(i).second;

        for (int i = 0; i < inputs.count(); i++) {
            QWebElement element = inputs.at(i);
            const QString typeAttr = element.attribute("type");

            if (!inputTypes.contains(typeAttr) && !typeAttr.isEmpty()) {
                continue;
            }

            if (key == element.attribute("name")) {
                completed = true;
                element.setAttribute("value", value);
            }
        }
    }
#endif

    return completed;
}

bool PageFormCompleter::queryItemsContains(const QueryItems &queryItems, const QString &attributeName,
        const QString &attributeValue) const
{
    if (attributeName.isEmpty() || attributeValue.isEmpty()) {
        return false;
    }

    for (int i = 0; i < queryItems.count(); i++) {
        const QueryItem item = queryItems.at(i);

        if (item.first == attributeName) {
            return item.second == attributeValue;
        }
    }

    return false;
}

QByteArray PageFormCompleter::convertWebKitFormBoundaryIfNecessary(const QByteArray &data) const
{
    /* Sometimes, data are passed in this format:
     *
     *  ------WebKitFormBoundary0bBp3bFMdGwqanMp
     *  Content-Disposition: form-data; name="name-of-attribute"
     *
     *  value-of-attribute
     *  ------WebKitFormBoundary0bBp3bFMdGwqanMp--
     *
     * So this function converts this format into name=value& format
     */

    if (!data.contains(QByteArray("------WebKitFormBoundary"))) {
        return data;
    }

    QByteArray formatedData;
    QzRegExp rx("name=\"(.*)------WebKitFormBoundary");
    rx.setMinimal(true);

    int pos = 0;
    while ((pos = rx.indexIn(data, pos)) != -1) {
        QString string = rx.cap(1);
        pos += rx.matchedLength();

        int endOfAttributeName = string.indexOf(QLatin1Char('"'));
        if (endOfAttributeName == -1) {
            continue;
        }

        QString attrName = string.left(endOfAttributeName);
        QString attrValue = string.mid(endOfAttributeName + 1).trimmed().remove(QLatin1Char('\n'));

        if (attrName.isEmpty() || attrValue.isEmpty()) {
            continue;
        }

        formatedData.append(attrName + "=" + attrValue + "&");
    }

    return formatedData;
}

PageFormCompleter::QueryItem PageFormCompleter::findUsername(const QWebElement &form) const
{
    Q_UNUSED(form)
#if QTWEBENGINE_DISABLED
    // Try to find username (or email) field in the form.
    QStringList selectors;
    selectors << "input[type=\"text\"][name*=\"user\"]"
              << "input[type=\"text\"][name*=\"name\"]"
              << "input[type=\"text\"]"
              << "input[type=\"email\"]"
              << "input:not([type=\"hidden\"][type=\"password\"])";

    foreach (const QString &selector, selectors) {
        const QWebElementCollection inputs = form.findAll(selector);
        foreach (QWebElement element, inputs) {
            const QString name = element.attribute("name");
            const QString value = element.evaluateJavaScript("this.value").toString();

            if (!name.isEmpty() && !value.isEmpty()) {
                QueryItem item;
                item.first = name;
                item.second = value;
                return item;
            }
        }
    }
#endif

    return QueryItem();
}

PageFormCompleter::QueryItems PageFormCompleter::createQueryItems(QByteArray data) const
{
    // QUrlQuery/QUrl never encodes/decodes + and spaces
    data.replace('+', ' ');

    QUrlQuery query;
    query.setQuery(data);
    QueryItems arguments = query.queryItems(QUrl::FullyDecoded);

    return arguments;
}

#if QTWEBENGINE_DISABLED
QWebElementCollection PageFormCompleter::getAllElementsFromPage(const QString &selector) const
{
    QWebElementCollection list;

    if (!m_page && !m_frame)
        return list;

    if (m_frame)
        return m_frame->findAllElements(selector);

    if (!m_page->mainFrame())
        return list;

    QList<QWebFrame*> frames;
    frames.append(m_page->mainFrame());

    while (!frames.isEmpty()) {
        QWebFrame* frame = frames.takeFirst();
        if (frame && !frame->documentElement().isNull()) {
            list.append(frame->findAllElements(selector));
            frames += frame->childFrames();
        }
    }

    return list;
}

#endif
