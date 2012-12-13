/* ============================================================
* Personal Information Manager plugin for QupZilla
* Copyright (C) 2012  David Rosca <nowrep@gmail.com>
* Copyright (C) 2012  Mladen Pejaković <pejakm@gmail.com>
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
#include "PIM_handler.h"
#include "PIM_settings.h"
#include "webview.h"
#include "webpage.h"

#include <QApplication>
#include <QSettings>
#include <QWebPage>
#include <QWebFrame>
#include <QLabel>
#include <QToolTip>
#include <QKeyEvent>

PIM_Handler::PIM_Handler(const QString &sPath, QObject* parent)
    : QObject(parent)
    , m_settingsFile(sPath + "extensions.ini")
    , m_loaded(false)
{
}

void PIM_Handler::loadSettings()
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);

    settings.beginGroup("PIM");
    m_allInfo[PI_LastName] = settings.value("LastName", QString()).toString();
    m_allInfo[PI_FirstName] = settings.value("FirstName", QString()).toString();
    m_allInfo[PI_Email] = settings.value("Email", QString()).toString();
    m_allInfo[PI_Mobile] = settings.value("Mobile", QString()).toString();
    m_allInfo[PI_Phone] = settings.value("Phone", QString()).toString();
    m_allInfo[PI_Address] = settings.value("Address", QString()).toString();
    m_allInfo[PI_City] = settings.value("City", QString()).toString();
    m_allInfo[PI_Zip] = settings.value("Zip", QString()).toString();
    m_allInfo[PI_State] = settings.value("State", QString()).toString();
    m_allInfo[PI_Country] = settings.value("Country", QString()).toString();
    m_allInfo[PI_HomePage] = settings.value("HomePage", QString()).toString();
    m_allInfo[PI_Special1] = settings.value("Special1", QString()).toString();
    m_allInfo[PI_Special2] = settings.value("Special2", QString()).toString();
    m_allInfo[PI_Special3] = settings.value("Special3", QString()).toString();
    settings.endGroup();

    m_translations[PI_LastName] = tr("Last Name");
    m_translations[PI_FirstName] = tr("First Name");
    m_translations[PI_Email] = tr("E-mail");
    m_translations[PI_Mobile] = tr("Mobile");
    m_translations[PI_Phone] = tr("Phone");
    m_translations[PI_Address] = tr("Address");
    m_translations[PI_City] = tr("City");
    m_translations[PI_Zip] = tr("ZIP Code");
    m_translations[PI_State] = tr("State/Region");
    m_translations[PI_Country] = tr("Country");
    m_translations[PI_HomePage] = tr("Home Page");
    m_translations[PI_Special1] = tr("Custom 1");
    m_translations[PI_Special2] = tr("Custom 2");
    m_translations[PI_Special3] = tr("Custom 3");

    m_infoMatches[PI_LastName] << "lastname" << "surname";
    m_infoMatches[PI_FirstName] << "firstname" << "name";
    m_infoMatches[PI_Email] << "email" << "e-mail" << "mail";
    m_infoMatches[PI_Mobile] << "mobile" << "mobilephone";
    m_infoMatches[PI_Phone] << "phone" << "telephone";
    m_infoMatches[PI_Address] << "address";
    m_infoMatches[PI_City] << "city";
    m_infoMatches[PI_Zip] << "zip";
    m_infoMatches[PI_State] << "state" << "region";
    m_infoMatches[PI_Country] << "country";
    m_infoMatches[PI_HomePage] << "homepage" << "www";

    m_loaded = true;
}

void PIM_Handler::showSettings(QWidget* parent)
{
    if (!m_settings) {
        m_settings = new PIM_Settings(m_settingsFile, parent);

        connect(m_settings.data(), SIGNAL(accepted()), this, SLOT(loadSettings()));
    }

    m_settings.data()->show();
    m_settings.data()->raise();
}

void PIM_Handler::populateWebViewMenu(QMenu* menu, WebView* view, const QWebHitTestResult &hitTest)
{
    m_view = view;
    m_element = hitTest.element();

    if (!hitTest.isContentEditable()) {
        return;
    }

    if (!m_loaded) {
        loadSettings();
    }

    QMenu* pimMenu = new QMenu(tr("Insert Personal Information"));
    pimMenu->setIcon(QIcon(":/PIM/data/PIM.png"));
    
    if (!m_allInfo[PI_FirstName].isEmpty() && !m_allInfo[PI_LastName].isEmpty()) {
        const QString fullname = m_allInfo[PI_FirstName] + " " + m_allInfo[PI_LastName];
        QAction* action = pimMenu->addAction(fullname, this, SLOT(pimInsert()));
        action->setData(fullname);
    }

    for (int i = 0; i < PI_Max; ++i) {
        const QString &info = m_allInfo[PI_Type(i)];
        if (info.isEmpty()) {
            continue;
        }

        QAction* action = pimMenu->addAction(info, this, SLOT(pimInsert()));
        action->setData(info);
        action->setStatusTip(m_translations[PI_Type(i)]);
    }

    pimMenu->addSeparator();
    pimMenu->addAction(tr("Edit"), this, SLOT(showSettings()));

    menu->addMenu(pimMenu);
    menu->addSeparator();
}

bool PIM_Handler::keyPress(WebView* view, QKeyEvent* event)
{
    if (!view) {
        return false;
    }

    bool isEnter = event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter;
    bool isControlModifier = event->modifiers() & Qt::ControlModifier;

    if (!isEnter || !isControlModifier) {
        return false;
    }

    const QWebElement &document = view->page()->mainFrame()->documentElement();
    const QWebElementCollection elements = document.findAll("input[type=\"text\"]");

    foreach(QWebElement element, elements) {
        const QString &name = element.attribute("name");
        if (name.isEmpty()) {
            continue;
        }

        PI_Type match = nameMatch(name);
        if (match != PI_Invalid) {
            element.evaluateJavaScript(QString("this.value = \"%1\"").arg(m_allInfo[match]));
        }
    }

    return true;
}

void PIM_Handler::unloadPlugin()
{
    delete m_settings.data();
}

void PIM_Handler::webPageCreated(WebPage* page)
{
    connect(page, SIGNAL(loadFinished(bool)), this, SLOT(pageLoadFinished()));
}

void PIM_Handler::pimInsert()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (m_element.isNull() || !action) {
        return;
    }

    QString info = action->data().toString();
    info.replace(QLatin1Char('"'), QLatin1String("\\\""));
    m_element.evaluateJavaScript(QString("var newVal = this.value.substring(0, this.selectionStart) + \"%1\" + this.value.substring(this.selectionEnd); this.value = newVal;").arg(info));
}

void PIM_Handler::pageLoadFinished()
{
    WebPage* page = qobject_cast<WebPage*>(sender());
    if (!page) {
        return;
    }

    if (!m_loaded) {
        loadSettings();
    }

    const QWebElement &document = page->mainFrame()->documentElement();
    const QWebElementCollection elements = document.findAll("input[type=\"text\"]");

    foreach(QWebElement element, elements) {
        const QString &name = element.attribute("name");
        if (name.isEmpty()) {
            continue;
        }

        PI_Type match = nameMatch(name);
        if (match != PI_Invalid) {
            element.setStyleProperty("-webkit-appearance", "none");
            element.setStyleProperty("-webkit-box-shadow", "inset 0 0 2px 1px #EEE000");
        }
    }
}

PIM_Handler::PI_Type PIM_Handler::nameMatch(const QString &name)
{
    for (int i = 0; i < PI_Max; ++i) {
        if (!m_allInfo[PI_Type(i)].isEmpty()) {
            foreach(const QString & n, m_infoMatches[PI_Type(i)]) {
                if (name == n) {
                    return PI_Type(i);
                }
                if (name.contains(n)) {
                    return PI_Type(i);
                }
            }
        }
    }

    return PI_Invalid;
}
