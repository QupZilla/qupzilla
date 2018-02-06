/* ============================================================
* KWalletPasswords - KWallet support plugin for QupZilla
* Copyright (C) 2013-2018 David Rosca <nowrep@gmail.com>
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
#include "kwalletplugin.h"
#include "kwalletpasswordbackend.h"
#include "pluginproxy.h"
#include "browserwindow.h"
#include "mainapplication.h"
#include "autofill.h"
#include "passwordmanager.h"

#include <QTranslator>

KWalletPlugin::KWalletPlugin()
    : QObject()
    , m_backend(0)
{
}

PluginSpec KWalletPlugin::pluginSpec()
{
    PluginSpec spec;
    spec.name = "KWallet Passwords";
    spec.info = "KWallet password backend";
    spec.description = "Provides support for storing passwords in KWallet";
    spec.version = "0.1.2";
    spec.author = "David Rosca <nowrep@gmail.com>";
    spec.icon = QPixmap(":kwp/data/icon.png");
    spec.hasSettings = false;

    return spec;
}

void KWalletPlugin::init(InitState state, const QString &settingsPath)
{
    Q_UNUSED(state);
    Q_UNUSED(settingsPath);

    m_backend = new KWalletPasswordBackend;
    mApp->autoFill()->passwordManager()->registerBackend(QSL("KWallet"), m_backend);
}

void KWalletPlugin::unload()
{
    mApp->autoFill()->passwordManager()->unregisterBackend(m_backend);
    delete m_backend;
}

bool KWalletPlugin::testPlugin()
{
    // Require the version that the plugin was built with
    return (Qz::VERSION == QLatin1String(QUPZILLA_VERSION));
}

QTranslator* KWalletPlugin::getTranslator(const QString &locale)
{
    QTranslator* translator = new QTranslator(this);
    translator->load(locale, ":/kwp/locale/");
    return translator;
}
