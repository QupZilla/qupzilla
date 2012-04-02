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
#include "testplugin.h"
#include "testplugin_sidebar.h"
#include "qupzilla.h"
#include "webview.h"
#include "pluginproxy.h"
#include "mainapplication.h"
#include "sidebar.h"

#include <QTranslator>
#include <QPushButton>
#include <QWebHitTestResult>
#include <QMenu>

TestPlugin::TestPlugin()
    : QObject()
    , m_view(0)
{
    // Don't do anything expensive in constructor!
    // It will be called even if user doesn't have
    // plugin allowed
}

PluginSpec TestPlugin::pluginSpec()
{
    PluginSpec spec;
    spec.name = "Example Plugin";
    spec.info = "Example minimal plugin";
    spec.description = "Very simple minimal plugin example";
    spec.version = "0.1.5";
    spec.author = "David Rosca <nowrep@gmail.com>";
    spec.icon = QPixmap(":qupzilla.png");
    spec.hasSettings = true;

    return spec;
}

void TestPlugin::init(const QString &sPath)
{
    qDebug() << __FUNCTION__ << "called";

    // This function is called right after plugin is loaded
    // it will be called even if we return false from testPlugin()
    // so it is recommended not to call any QupZilla function here

    // Settings path is PROFILE/extensions/, in this directory
    // you can use global .ini file for QSettings named "extensions.ini"
    // or create new folder for your plugin and save in it anything you want
    m_settingsPath = sPath;

    // Registering this plugin as a MousePressHandler.
    // Otherwise mousePress() function will never be called
    QZ_REGISTER_EVENT_HANDLER(PluginProxy::MousePressHandler);

    // Adding new sidebar into application
    SideBarManager::addSidebar("testplugin-sidebar", new TestPlugin_Sidebar(this));
}

void TestPlugin::unload()
{
    qDebug() << __FUNCTION__ << "called";

    // This function will be called when unloading plugin
    // it will be also called if we return false from testPlugin()

    // Removing sidebar from application
    SideBarManager::removeSidebar("testplugin-sidebar");
}

bool TestPlugin::testPlugin()
{
    //This function is called right after init()
    //There should be some testing if plugin is loaded correctly
    //If this function returns false, plugin is automatically unloaded

    return (QupZilla::VERSION == "1.1.8");
}

QTranslator* TestPlugin::getTranslator(const QString &locale)
{
    // Loads translation according to locale file
    // QString locale will contains "fr_FR.qm" for French locale

    QTranslator* translator = new QTranslator(this);
    translator->load(":/testplugin/locale/" + locale);
    return translator;
}

void TestPlugin::showSettings(QWidget* parent)
{
    // This function will be called from Preferences after clicking on Settings button.
    // Settings button will be enabled if PluginSpec.hasSettings == true

    QDialog* dialog = new QDialog(parent);
    QPushButton* b = new QPushButton("Example Plugin v0.0.1");
    QPushButton* closeButton = new QPushButton(tr("Close"));
    QLabel* label = new QLabel();
    label->setPixmap(QPixmap(":icons/other/about.png"));

    QVBoxLayout* l = new QVBoxLayout(dialog);
    l->addWidget(label);
    l->addWidget(b);
    l->addWidget(closeButton);
    dialog->setLayout(l);

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(tr("Example Plugin Settings"));
    dialog->setWindowIcon(QIcon(":qupzilla.png"));
    connect(closeButton, SIGNAL(clicked()), dialog, SLOT(close()));

    dialog->show();
}

void TestPlugin::populateWebViewMenu(QMenu* menu, WebView* view, const QWebHitTestResult &r)
{
    // Called from WebView when creating context menu

    m_view = view;

    QString title;
    if (!r.imageUrl().isEmpty()) {
        title += " on image";
    }

    if (!r.linkUrl().isEmpty()) {
        title += " on link";
    }

    if (r.isContentEditable()) {
        title += " on input";
    }

    menu->addAction(tr("My first plugin action") + title, this, SLOT(actionSlot()));
}

bool TestPlugin::mousePress(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event)
{
    qDebug() << "mousePress" << type << obj << event;

    // Returning false means, that we don't want to block propagating this event
    // Returning true may affect behaviour of QupZilla, so make sure you know what
    // you are doing!
    return false;
}

void TestPlugin::actionSlot()
{
    QMessageBox::information(m_view, tr("Hello"), tr("First plugin action works :-)"));
}

// Export plugin macro
// This macro has to be only in class derived from PluginInterface
// Don't call it in other files!
Q_EXPORT_PLUGIN2(ExamplePlugin, TestPlugin)
