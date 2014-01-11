/* ============================================================
* StatusBarIcons - Extra icons in statusbar for QupZilla
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
#include "sbi_imagesicon.h"
#include "mainapplication.h"
#include "qupzilla.h"
#include "tabwidget.h"
#include "tabbedwebview.h"
#include "webpage.h"

#include <QGraphicsColorizeEffect>
#include <QWebSettings>
#include <QSettings>
#include <QMenu>

SBI_ImagesIcon::SBI_ImagesIcon(QupZilla* window, const QString &settingsPath)
    : ClickableLabel(window)
    , p_QupZilla(window)
    , m_settingsFile(settingsPath + "extensions.ini")
{
    setCursor(Qt::PointingHandCursor);
    setToolTip(tr("Modify images loading settings per-site and globally"));

    m_icon = QIcon::fromTheme("image-x-generics", QIcon(":sbi/data/images.png"));
    setPixmap(m_icon.pixmap(16));

    QSettings settings(m_settingsFile, QSettings::IniFormat);
    settings.beginGroup("StatusBarIcons_Images");
    m_loadingImages = settings.value("LoadImages", true).toBool();
    settings.endGroup();

    mApp->webSettings()->setAttribute(QWebSettings::AutoLoadImages, m_loadingImages);

    updateIcon();

    connect(p_QupZilla->tabWidget(), SIGNAL(currentChanged(int)), this, SLOT(updateIcon()));
    connect(this, SIGNAL(clicked(QPoint)), this, SLOT(showMenu(QPoint)));
}

void SBI_ImagesIcon::showMenu(const QPoint &point)
{
    QFont boldFont = font();
    boldFont.setBold(true);

    QMenu menu;
    menu.addAction(m_icon, tr("Current page settings"))->setFont(boldFont);

    if (currentPageSettings()->testAttribute(QWebSettings::AutoLoadImages)) {
        menu.addAction(tr("Disable loading images (temporarily)"), this, SLOT(toggleLoadingImages()));
    }
    else {
        menu.addAction(tr("Enable loading images (temporarily)"), this, SLOT(toggleLoadingImages()));
    }

    menu.addSeparator();
    menu.addAction(m_icon, tr("Global settings"))->setFont(boldFont);

    QAction* act = menu.addAction(tr("Automatically load images"));
    act->setCheckable(true);
    act->setChecked(m_loadingImages);
    connect(act, SIGNAL(toggled(bool)), this, SLOT(setGlobalLoadingImages(bool)));

    menu.exec(point);
}

void SBI_ImagesIcon::toggleLoadingImages()
{
    bool current = currentPageSettings()->testAttribute(QWebSettings::AutoLoadImages);
    currentPageSettings()->setAttribute(QWebSettings::AutoLoadImages, !current);

    // We should reload page on disabling images
    if (current) {
        p_QupZilla->weView()->reload();
    }

    updateIcon();
}

void SBI_ImagesIcon::setGlobalLoadingImages(bool enable)
{
    // Save it permanently
    QSettings settings(m_settingsFile, QSettings::IniFormat);
    settings.beginGroup("StatusBarIcons_Images");
    settings.setValue("LoadImages", enable);
    settings.endGroup();

    // Switch it in websettings
    m_loadingImages = enable;
    mApp->webSettings()->setAttribute(QWebSettings::AutoLoadImages, m_loadingImages);
    updateIcon();

    // We should reload page on disabling images
    if (!enable) {
        p_QupZilla->weView()->reload();
    }
}

QWebSettings* SBI_ImagesIcon::currentPageSettings()
{
    return p_QupZilla->weView()->page()->settings();
}

void SBI_ImagesIcon::updateIcon()
{
    if (currentPageSettings()->testAttribute(QWebSettings::AutoLoadImages)) {
        setGraphicsEffect(0);
    }
    else {
        QGraphicsColorizeEffect* effect = new QGraphicsColorizeEffect(this);
        effect->setColor(Qt::gray);
        setGraphicsEffect(effect);
    }
}
