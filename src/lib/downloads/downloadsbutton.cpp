/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2018 David Rosca <nowrep@gmail.com>
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
#include "downloadsbutton.h"
#include "mainapplication.h"
#include "downloadmanager.h"

DownloadsButton::DownloadsButton(QObject *parent)
    : AbstractButtonInterface(parent)
    , m_manager(mApp->downloadManager())
{
    setIcon(QIcon::fromTheme(QSL("edit-download"), QIcon(QSL(":icons/menu/download.svg"))));
    setTitle(tr("Downloads"));
    setToolTip(tr("Open Download Manager"));

    connect(this, &AbstractButtonInterface::clicked, this, &DownloadsButton::clicked);
    connect(m_manager, &DownloadManager::downloadsCountChanged, this, &DownloadsButton::updateState);

    updateState();
}

QString DownloadsButton::id() const
{
    return QSL("button-downloads");
}

QString DownloadsButton::name() const
{
    return tr("Downloads");
}

void DownloadsButton::updateState()
{
    setVisible(m_manager->downloadsCount() > 0);
    const int count = m_manager->activeDownloadsCount();
    if (count > 0) {
        setBadgeText(QString::number(count));
    } else {
        setBadgeText(QString());
    }
}

void DownloadsButton::clicked(ClickController *controller)
{
    Q_UNUSED(controller)

    mApp->downloadManager()->show();
}
