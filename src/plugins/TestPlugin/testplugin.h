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
#ifndef TESTPLUGIN_H
#define TESTPLUGIN_H

// Include plugininterface.h for your version of QupZilla
#include "plugininterface.h"

#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QPointer>

class TestPlugin_Sidebar;

class TestPlugin : public QObject, public PluginInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginInterface)
    Q_PLUGIN_METADATA(IID "QupZilla.Browser.plugin.TestPlugin")

public:
    explicit TestPlugin();
    PluginSpec pluginSpec();

    void init(InitState state, const QString &settingsPath);
    void unload();
    bool testPlugin();

    QTranslator* getTranslator(const QString &locale);
    void showSettings(QWidget* parent = 0);

    void populateWebViewMenu(QMenu* menu, WebView* view, const WebHitTestResult &r);

    bool mousePress(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event);

private slots:
    void actionSlot();

private:
    QPointer<QDialog> m_settings;

    WebView* m_view;
    QString m_settingsPath;
    TestPlugin_Sidebar *m_sideBar = nullptr;
};

#endif // TESTPLUGIN_H
