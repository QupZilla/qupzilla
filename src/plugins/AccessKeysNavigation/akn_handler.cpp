/* ============================================================
* Access Keys Navigation plugin for QupZilla
* Copyright (C) 2012-2014  David Rosca <nowrep@gmail.com>
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
/*
 * Copyright 2008-2009 Benjamin C. Meyer <ben@meyerhome.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */
#include "akn_handler.h"
#include "webview.h"
#include "webpage.h"

#include <QApplication>
#include <QSettings>
#include <QWebPage>
#include <QWebFrame>
#include <QLabel>
#include <QTimer>
#include <QToolTip>
#include <QKeyEvent>

Qt::Key keyFromCode(int code)
{
    switch (code) {
    case 0:
        return Qt::Key_Control;

    case 1:
        return Qt::Key_Alt;

    case 2:
        return Qt::Key_Shift;

    default:
        // Using default shortcut
        return Qt::Key_Control;
    }
}

AKN_Handler::AKN_Handler(const QString &sPath, QObject* parent)
    : QObject(parent)
    , m_accessKeysVisible(false)
    , m_settingsFile(sPath + "/extensions.ini")
{
    loadSettings();
}

QString AKN_Handler::settingsFile()
{
    return m_settingsFile;
}

void AKN_Handler::loadSettings()
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);

    settings.beginGroup("AccessKeysNavigation");
    m_key = keyFromCode(settings.value("Key", 0).toInt());
    m_isDoublePress = settings.value("DoublePress", true).toBool();
    settings.endGroup();
}

bool AKN_Handler::handleKeyPress(QObject* obj, QKeyEvent* event)
{
    WebView* view = qobject_cast<WebView*>(obj);
    if (!view) {
        return false;
    }

    if (m_accessKeysVisible) {
        handleAccessKey(event);
        return true;
    }

    if (event->key() != m_key) {
        m_lastKeyPressTime = QTime();
        return false;
    }

    m_view = view;

    if (!m_isDoublePress) {
        triggerShowAccessKeys();
    }
    else {
        if (m_lastKeyPressTime.isNull()) {
            // It is the first press of our button
            m_lastKeyPressTime.start();
        }
        else {
            const int doublePressInterval = 500; // 500 msecs

            // It is the second press of our button
            if (m_lastKeyPressTime.elapsed() <= doublePressInterval) {
                triggerShowAccessKeys();
            }
            else {
                m_lastKeyPressTime = QTime::currentTime();
            }
        }
    }

    return false;
}

bool AKN_Handler::eventFilter(QObject* obj, QEvent* event)
{
    if (obj != m_view.data()) {
        return false;
    }

    switch (event->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::Resize:
    case QEvent::FocusOut:
    case QEvent::Wheel:
        hideAccessKeys();
        break;

    default:
        break;
    }

    return false;
}

void AKN_Handler::triggerShowAccessKeys()
{
    if (m_accessKeysVisible) {
        hideAccessKeys();
    }
    else {
        QTimer::singleShot(0, this, SLOT(showAccessKeys()));
    }
}

void AKN_Handler::handleAccessKey(QKeyEvent* event)
{
    if (event->key() == m_key) {
        hideAccessKeys();
        return;
    }

    QString text = event->text();
    if (text.isEmpty()) {
        return;
    }

    if (!m_view) {
        return;
    }

    QChar key = text.at(0);

    QChar other(QChar::Null);
    if (key.isLower()) {
        other = key.toUpper();
    }
    else if (key.isUpper()) {
        other = key.toLower();
    }

    if (!other.isNull() &&
        m_accessKeyNodes.contains(other) &&
        !m_accessKeyNodes.contains(key)
       ) {
        key = other;
    }

    if (m_accessKeyNodes.contains(key)) {
        QWebElement element = m_accessKeyNodes[key];
        QPoint p = element.geometry().center();
        QWebFrame* frame = element.webFrame();

        if (!frame) {
            return;
        }

        do {
            p -= frame->scrollPosition();
            frame = frame->parentFrame();
        }
        while (frame && frame != m_view.data()->page()->currentFrame());

        QMouseEvent pevent(QEvent::MouseButtonPress, p, Qt::LeftButton, 0, 0);
        qApp->sendEvent(m_view.data(), &pevent);

        QMouseEvent revent(QEvent::MouseButtonRelease, p, Qt::LeftButton, 0, 0);
        qApp->sendEvent(m_view.data(), &revent);

        hideAccessKeys();
    }
}

void AKN_Handler::showAccessKeys()
{
    if (!m_view) {
        return;
    }

    QWebPage* page = m_view.data()->page();

    QStringList supportedElement;
    supportedElement << QLatin1String("input")
                     << QLatin1String("a")
                     << QLatin1String("area")
                     << QLatin1String("button")
                     << QLatin1String("label")
                     << QLatin1String("legend")
                     << QLatin1String("textarea");

    QList<QChar> unusedKeys;
    for (char c = 'A'; c <= 'Z'; ++c) {
        unusedKeys << QLatin1Char(c);
    }
    for (char c = '0'; c <= '9'; ++c) {
        unusedKeys << QLatin1Char(c);
    }
    for (char c = 'a'; c <= 'z'; ++c) {
        unusedKeys << QLatin1Char(c);
    }

    QRect viewport = QRect(page->currentFrame()->scrollPosition(), page->viewportSize());
    // Priority first goes to elements with accesskey attributes
    QList<QWebElement> alreadyLabeled;
    foreach (const QString &elementType, supportedElement) {
        QList<QWebElement> result = page->currentFrame()->findAllElements(elementType).toList();
        foreach (const QWebElement &element, result) {
            const QRect geometry = element.geometry();
            if (geometry.size().isEmpty() ||
                !viewport.contains(geometry.topLeft())
               ) {
                continue;
            }
            QString accessKeyAttribute = element.attribute(QLatin1String("accesskey")).toUpper();
            if (accessKeyAttribute.isEmpty()) {
                continue;
            }
            QChar accessKey;
            for (int i = 0; i < accessKeyAttribute.count(); i += 2) {
                const QChar possibleAccessKey = accessKeyAttribute[i];
                if (unusedKeys.contains(possibleAccessKey)) {
                    accessKey = possibleAccessKey;
                    break;
                }
            }
            if (accessKey.isNull()) {
                continue;
            }
            unusedKeys.removeOne(accessKey);
            makeAccessKeyLabel(accessKey, element);
            alreadyLabeled.append(element);
        }
    }

    // Pick an access key first from the letters in the text and then from the
    // list of unused access keys
    foreach (const QString &elementType, supportedElement) {
        QWebElementCollection result = page->currentFrame()->findAllElements(elementType);
        foreach (const QWebElement &element, result) {
            const QRect geometry = element.geometry();
            if (unusedKeys.isEmpty() ||
                alreadyLabeled.contains(element) ||
                geometry.size().isEmpty() ||
                !viewport.contains(geometry.topLeft())
               ) {
                continue;
            }
            QChar accessKey;
            QString text = element.toPlainText().toLower();
            for (int i = 0; i < text.count(); ++i) {
                const QChar c = text.at(i);
                if (unusedKeys.contains(c)) {
                    accessKey = c;
                    break;
                }
            }
            if (accessKey.isNull()) {
                accessKey = unusedKeys.takeFirst();
            }
            unusedKeys.removeOne(accessKey);
            makeAccessKeyLabel(accessKey, element);
        }
    }

    // Install event filter and connect loadStarted
    m_accessKeysVisible = !m_accessKeyLabels.isEmpty();
    if (m_accessKeysVisible) {
        m_view.data()->installEventFilter(this);
        connect(m_view.data(), SIGNAL(loadStarted()), this, SLOT(hideAccessKeys()));
        connect(m_view.data()->page(), SIGNAL(scrollRequested(int,int,QRect)), this, SLOT(hideAccessKeys()));
#if QT_VERSION >= 0x040800
        connect(m_view.data()->page(), SIGNAL(viewportChangeRequested()), this, SLOT(hideAccessKeys()));
#endif
    }
}

void AKN_Handler::hideAccessKeys()
{
    if (!m_accessKeyLabels.isEmpty() && m_view) {
        // Fixes crash when hiding labels while closing view
        if (!m_view->inherits("WebView")) {
            m_accessKeyLabels.clear();
            m_accessKeyNodes.clear();
            return;
        }

        for (int i = 0; i < m_accessKeyLabels.count(); ++i) {
            QLabel* label = m_accessKeyLabels[i];
            label->hide();
            label->deleteLater();
        }
        m_accessKeyLabels.clear();
        m_accessKeyNodes.clear();
        m_view.data()->update();

        // Uninstall event filter and disconnect loadStarted
        m_view.data()->removeEventFilter(this);
        disconnect(m_view.data(), SIGNAL(loadStarted()), this, SLOT(hideAccessKeys()));
        disconnect(m_view.data()->page(), SIGNAL(scrollRequested(int,int,QRect)), this, SLOT(hideAccessKeys()));
#if QT_VERSION >= 0x040800
        disconnect(m_view.data()->page(), SIGNAL(viewportChangeRequested()), this, SLOT(hideAccessKeys()));
#endif
    }

    m_accessKeysVisible = false;
}

void AKN_Handler::makeAccessKeyLabel(const QChar &accessKey, const QWebElement &element)
{
    QLabel* label = new QLabel(m_view.data()->overlayWidget());
    label->setText(QString(QLatin1String("<b>%1</b>")).arg(accessKey));

    QPalette p = QToolTip::palette();
    QColor color = QColor(220, 243, 253);
    color.setAlpha(175);
    p.setColor(QPalette::Window, color);
    p.setColor(QPalette::WindowText, Qt::black);

    label->setPalette(p);
    label->setAutoFillBackground(true);
    label->setFrameStyle(QFrame::Box | QFrame::Plain);
    QPoint point = element.geometry().center();
    point -= m_view.data()->page()->currentFrame()->scrollPosition();
    label->show();
    label->resize(label->sizeHint());
    point.setX(point.x() - label->width() / 2);
    label->move(point);
    m_accessKeyLabels.append(label);
    m_accessKeyNodes[accessKey] = element;
}
