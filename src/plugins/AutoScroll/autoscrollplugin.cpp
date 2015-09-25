/* ============================================================
* AutoScroll - Autoscroll for QupZilla
* Copyright (C) 2014  David Rosca <nowrep@gmail.com>
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
#include "autoscrollplugin.h"
#include "autoscrollsettings.h"
#include "mainapplication.h"
#include "qztools.h"

#include <QSettings>
#include <QTranslator>
#include <QWebEngineProfile>
#include <QWebEngineScriptCollection>

AutoScrollPlugin::AutoScrollPlugin()
    : QObject()
{
}

PluginSpec AutoScrollPlugin::pluginSpec()
{
    PluginSpec spec;
    spec.name = QSL("AutoScroll");
    spec.info = QSL("AutoScroll plugin");
    spec.description = QSL("Provides support for autoscroll");
    spec.version = QSL("0.2.0");
    spec.author = QSL("David Rosca <nowrep@gmail.com>");
    spec.icon = QPixmap(QSL(":/autoscroll/data/scroll_all.png"));
    spec.hasSettings = true;

    return spec;
}

void AutoScrollPlugin::init(InitState state, const QString &settingsPath)
{
    Q_UNUSED(state)

    m_settingsPath = settingsPath;

    updateScript();
}

void AutoScrollPlugin::unload()
{
    QWebEngineScript script = mApp->webProfile()->scripts()->findScript(QSL("_qupzilla_autoscroll"));
    if (!script.isNull()) {
        mApp->webProfile()->scripts()->remove(script);
    }
}

bool AutoScrollPlugin::testPlugin()
{
    // Require the version that the plugin was built with
    return (Qz::VERSION == QLatin1String(QUPZILLA_VERSION));
}

QTranslator *AutoScrollPlugin::getTranslator(const QString &locale)
{
    QTranslator* translator = new QTranslator(this);
    translator->load(locale, QSL(":/autoscroll/locale/"));
    return translator;
}

void AutoScrollPlugin::showSettings(QWidget *parent)
{
    if (!m_settings) {
        m_settings = new AutoScrollSettings(m_settingsPath, parent);
        connect(m_settings, &AutoScrollSettings::settingsChanged, this, &AutoScrollPlugin::updateScript);
    }

    m_settings.data()->show();
    m_settings.data()->raise();
}

void AutoScrollPlugin::updateScript()
{
    const QString name = QSL("_qupzilla_autoscroll");

    QWebEngineScript oldScript = mApp->webProfile()->scripts()->findScript(name);
    if (!oldScript.isNull()) {
        mApp->webProfile()->scripts()->remove(oldScript);
    }

    QWebEngineScript script;
    script.setName(name);
    script.setInjectionPoint(QWebEngineScript::DocumentCreation);
    script.setWorldId(QWebEngineScript::ApplicationWorld);
    script.setRunsOnSubFrames(false);

    QSettings settings(m_settingsPath + QL1S("/extensions.ini"), QSettings::IniFormat);
    settings.beginGroup(QSL("AutoScroll"));

    QString source = QzTools::readAllFileContents(QSL(":/autoscroll/data/autoscroll.js"));
    source.replace(QSL("%MOVE_SPEED%"), settings.value(QSL("Speed"), 5).toString());
    source.replace(QSL("%CTRL_CLICK%"), settings.value(QSL("CtrlClick"), true).toString());
    source.replace(QSL("%MIDDLE_CLICK%"), settings.value(QSL("MiddleClick"), true).toString());
    source.replace(QSL("%IMG_ALL%"), QzTools::pixmapToDataUrl(QPixmap(QSL(":/autoscroll/data/scroll_all.png"))).toString());
    source.replace(QSL("%IMG_HORIZONTAL%"), QzTools::pixmapToDataUrl(QPixmap(QSL(":/autoscroll/data/scroll_horizontal.png"))).toString());
    source.replace(QSL("%IMG_VERTICAL%"), QzTools::pixmapToDataUrl(QPixmap(QSL(":/autoscroll/data/scroll_vertical.png"))).toString());

    script.setSourceCode(source);

    mApp->webProfile()->scripts()->insert(script);
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(AutoScroll, AutoScrollPlugin)
#endif
