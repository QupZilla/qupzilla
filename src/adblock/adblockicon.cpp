/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca
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
#include "adblockicon.h"
#include "adblockmanager.h"
#include "qupzilla.h"
#include "webpage.h"

AdBlockIcon::AdBlockIcon(QupZilla* mainClass, QWidget* parent)
   : ClickableLabel(parent)
   , p_QupZilla(mainClass)
{
    setMaximumHeight(16);
    setCursor(Qt::PointingHandCursor);
    setToolTip(tr("AdBlock let you block any unwanted content on pages"));

    connect(this, SIGNAL(clicked(QPoint)), this, SLOT(showMenu(QPoint)));
}

void AdBlockIcon::showMenu(const QPoint &pos)
{
    AdBlockManager* manager = AdBlockManager::instance();

    QMenu menu;
    menu.addAction(tr("Show AdBlock &Settings"), manager, SLOT(showDialog()));
    menu.addSeparator();
    QList<WebPage::AdBlockedEntry> entries = p_QupZilla->weView()->webPage()->adBlockedEntries();
    if (entries.isEmpty())
        menu.addAction(tr("No content blocked"))->setEnabled(false);
    else {
        menu.addAction(tr("Blocked URL (AdBlock Rule) - click to edit rule"))->setEnabled(false);
        foreach (WebPage::AdBlockedEntry entry, entries) {
            QString address = entry.url.toString().right(55);
            menu.addAction(tr("%1 with (%2)").arg(address, entry.rule), manager, SLOT(showRule()))->setData(entry.rule);
        }
    }
    menu.addSeparator();
    menu.addAction(tr("Learn About Writing &Rules"), this, SLOT(learnAboutRules()));

    menu.exec(pos);
}

void AdBlockIcon::learnAboutRules()
{
    p_QupZilla->tabWidget()->addView(QUrl("http://adblockplus.org/en/filters"), tr("New tab"), TabWidget::NewSelectedTab);
}

void AdBlockIcon::setEnabled(bool enabled)
{
    if (enabled)
        setPixmap(QPixmap(":icons/other/adblock.png"));
    else
        setPixmap(QPixmap(":icons/other/adblock-disabled.png"));
}

AdBlockIcon::~AdBlockIcon()
{
}
