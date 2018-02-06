/* ============================================================
* GnomeKeyringPasswords - gnome-keyring support plugin for QupZilla
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
#include "gnomekeyringplugin.h"
#include "gnomekeyringpasswordbackend.h"
#include "pluginproxy.h"
#include "browserwindow.h"
#include "mainapplication.h"
#include "autofill.h"
#include "passwordmanager.h"

#include <QTranslator>

GnomeKeyringPlugin::GnomeKeyringPlugin()
    : QObject()
    , m_backend(0)
{
}

PluginSpec GnomeKeyringPlugin::pluginSpec()
{
    PluginSpec spec;
    spec.name = "Gnome Keyring Passwords";
    spec.info = "Gnome Keyring password backend";
    spec.description = "Provides support for storing passwords in gnome-keyring";
    spec.version = "0.1.0";
    spec.author = "David Rosca <nowrep@gmail.com>";
    spec.icon = QPixmap(":gkp/data/icon.png");
    spec.hasSettings = false;

    return spec;
}

void GnomeKeyringPlugin::init(InitState state, const QString &settingsPath)
{
    Q_UNUSED(state);
    Q_UNUSED(settingsPath);

    m_backend = new GnomeKeyringPasswordBackend;
    mApp->autoFill()->passwordManager()->registerBackend(QSL("GnomeKeyring"), m_backend);
}

void GnomeKeyringPlugin::unload()
{
    mApp->autoFill()->passwordManager()->unregisterBackend(m_backend);
    delete m_backend;
}

bool GnomeKeyringPlugin::testPlugin()
{
    // Require the version that the plugin was built with
    return (Qz::VERSION == QLatin1String(QUPZILLA_VERSION));
}

QTranslator* GnomeKeyringPlugin::getTranslator(const QString &locale)
{
    QTranslator* translator = new QTranslator(this);
    translator->load(locale, ":/gkp/locale/");
    return translator;
}
