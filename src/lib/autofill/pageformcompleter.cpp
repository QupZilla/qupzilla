/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2013  David Rosca <nowrep@gmail.com>
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

#include <QWebPage>
#include <QWebFrame>
#include <QWebElement>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif
#include <QDebug>

PageFormCompleter::PageFormCompleter(QWebPage* page)
    : m_page(page)
{
}

PageFormData PageFormCompleter::extractFormData(const QByteArray &postData) const
{
    QString usernameName;
    QString usernameValue;
    QString passwordName;
    QString passwordValue;

    QWebElementCollection allForms;
    QWebElement foundForm;

    QByteArray data = convertWebKitFormBoundaryIfNecessary(postData);
    PageFormData formData = {false, QString(), QString(), data};

    if (data.isEmpty()) {
        return formData;
    }

    if (!data.contains('=')) {
        qDebug() << "PageFormCompleter: Invalid form data" << data;
        return formData;
    }

    /* Find all form elements in page (in all frames) */
    QList<QWebFrame*> frames;
    frames.append(m_page->mainFrame());
    while (!frames.isEmpty()) {
        QWebFrame* frame = frames.takeFirst();
        allForms.append(frame->findAllElements("form"));
        frames += frame->childFrames();
    }

    const QueryItems &queryItems = createQueryItems(data);

    /* Find form that contains password value sent in data */
    foreach(const QWebElement & formElement, allForms) {
        const QWebElementCollection &inputs = formElement.findAll("input[type=\"password\"]");
        foreach(QWebElement inputElement, inputs) {
            passwordName = inputElement.attribute("name");
            passwordValue = inputElement.evaluateJavaScript("this.value").toString();

            if (queryItemsContains(queryItems, passwordName, passwordValue)) {
                foundForm = formElement;
                break;
            }
        }

        if (!foundForm.isNull()) {
            break;
        }
    }

    if (foundForm.isNull()) {
        return formData;
    }

    /* Try to find username (or email) field in the form. */
    bool found = false;
    QStringList selectors;
    selectors << "input[type=\"text\"][name*=\"user\"]"
              << "input[type=\"text\"][name*=\"name\"]"
              << "input[type=\"text\"]"
              << "input[type=\"email\"]"
              << "input:not([type=\"hidden\"][type=\"password\"])";

    foreach(const QString & selector, selectors) {
        const QWebElementCollection &inputs = foundForm.findAll(selector);
        foreach(QWebElement element, inputs) {
            usernameName = element.attribute("name");
            usernameValue = element.evaluateJavaScript("this.value").toString();

            if (!usernameName.isEmpty() && !usernameValue.isEmpty()) {
                found = true;
                break;
            }
        }

        if (found) {
            break;
        }
    }

    formData.found = true;
    formData.username = usernameValue;
    formData.password = passwordValue;

    return formData;
}

void PageFormCompleter::completePage(const QByteArray &data) const
{
    const QueryItems &queryItems = createQueryItems(data);

    /* Input types that are being completed */
    QStringList inputTypes;
    inputTypes << "text" << "password" << "email";

    /* Find all input elements in the page */
    QWebElementCollection inputs;
    QList<QWebFrame*> frames;
    frames.append(m_page->mainFrame());
    while (!frames.isEmpty()) {
        QWebFrame* frame = frames.takeFirst();
        inputs.append(frame->findAllElements("input"));
        frames += frame->childFrames();
    }

    for (int i = 0; i < queryItems.count(); i++) {
        const QString &key = queryItems.at(i).first;
        const QString &value = queryItems.at(i).second;

        /* Is it really necessary?
        key = QUrl::fromEncoded(key.toUtf8()).toString();
        value = QUrl::fromEncoded(value.toUtf8()).toString();
        */

        for (int i = 0; i < inputs.count(); i++) {
            QWebElement element = inputs.at(i);
            const QString &typeAttr = element.attribute("type");

            if (!inputTypes.contains(typeAttr) && !typeAttr.isEmpty()) {
                continue;
            }

            if (key == element.attribute("name")) {
                element.setAttribute("value", value);
            }
        }
    }
}

bool PageFormCompleter::queryItemsContains(const QueryItems &queryItems, const QString &attributeName,
        const QString &attributeValue) const
{
    if (attributeName.isEmpty() || attributeValue.isEmpty()) {
        return false;
    }

    for (int i = 0; i < queryItems.count(); i++) {
        const QueryItem &item = queryItems.at(i);

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
    QRegExp rx("name=\"(.*)------WebKitFormBoundary");
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

PageFormCompleter::QueryItems PageFormCompleter::createQueryItems(const QByteArray &data) const
{
    /* Why not to use encodedQueryItems = QByteArrays ?
     * Because we need to filter "+" characters that must be spaces
     * (not real "+" characters "%2B")
     *
     * DO NOT TOUCH! It works now with both Qt 4 & Qt 5 ...
     */
#if QT_VERSION >= 0x050000
    QueryItems arguments = QUrlQuery(QUrl::fromEncoded("http://foo.com/?" + data)).queryItems();
#else
    QByteArray dataCopy = data;
    dataCopy.replace('+', ' ');
    QueryItems arguments = QUrl::fromEncoded("http://foo.com/?" + dataCopy).queryItems();
#endif

#if QT_VERSION >= 0x050000
    for (int i = 0; i < arguments.count(); i++) {
        arguments[i].first.replace(QLatin1Char('+'), QLatin1Char(' '));
        arguments[i].second.replace(QLatin1Char('+'), QLatin1Char(' '));
    }
#endif

    return arguments;
}
