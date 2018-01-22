/* ============================================================
* StatusBarIcons - Extra icons in statusbar for QupZilla
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
#include "sbi_javascripticon.h"
#include "browserwindow.h"
#include "tabwidget.h"
#include "tabbedwebview.h"
#include "webpage.h"
#include "jsoptions.h"

#include <QGraphicsColorizeEffect>
#include <QWebEngineSettings>
#include <QMenu>

SBI_JavaScriptIcon::SBI_JavaScriptIcon(BrowserWindow* window)
    : SBI_Icon(window)
{
    setObjectName(QSL("sbi_javascripticon"));
    setCursor(Qt::PointingHandCursor);
    setToolTip(tr("Modify JavaScript settings per-site and globally"));

    m_icon = QIcon::fromTheme("application-x-javascript", QIcon(":sbi/data/javascript.png"));
    setPixmap(m_icon.pixmap(16));

    connect(m_window->tabWidget(), SIGNAL(currentChanged(int)), this, SLOT(updateIcon()));
    connect(this, SIGNAL(clicked(QPoint)), this, SLOT(showMenu(QPoint)));

    updateIcon();
}

void SBI_JavaScriptIcon::showMenu(const QPoint &point)
{
    QFont boldFont = font();
    boldFont.setBold(true);

    QMenu menu;
    menu.addAction(m_icon, tr("Current Page Settings"))->setFont(boldFont);

    if (testCurrentPageWebAttribute(QWebEngineSettings::JavascriptEnabled)) {
        menu.addAction(tr("Disable JavaScript (temporarily)"), this, SLOT(toggleJavaScript()));
    }
    else {
        menu.addAction(tr("Enable JavaScript (temporarily)"), this, SLOT(toggleJavaScript()));
    }

    // JavaScript needs to be always enabled for qupzilla: sites
    if (currentPage() && currentPage()->url().scheme() == QLatin1String("qupzilla")) {
        menu.actions().at(1)->setEnabled(false);
    }

    menu.addSeparator();
    menu.addAction(m_icon, tr("Global Settings"))->setFont(boldFont);
    menu.addAction(tr("Manage JavaScript settings"), this, SLOT(openJavaScriptSettings()));
    menu.exec(point);
}

void SBI_JavaScriptIcon::updateIcon()
{
    if (testCurrentPageWebAttribute(QWebEngineSettings::JavascriptEnabled)) {
        setGraphicsEffect(0);
    }
    else {
        QGraphicsColorizeEffect* effect = new QGraphicsColorizeEffect(this);
        effect->setColor(Qt::gray);
        setGraphicsEffect(effect);
    }
}

void SBI_JavaScriptIcon::toggleJavaScript()
{
    WebPage *page = currentPage();
    if (!page) {
        return;
    }

    bool current = testCurrentPageWebAttribute(QWebEngineSettings::JavascriptEnabled);
    setCurrentPageWebAttribute(QWebEngineSettings::JavascriptEnabled, !current);

    m_settings[page] = !current;

    connect(page, &WebPage::navigationRequestAccepted, this, [=](const QUrl &, WebPage::NavigationType, bool isMainFrame) {
        if (isMainFrame) {
            page->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, m_settings[page]);
        }
    });

    m_window->weView()->reload();

    updateIcon();
}

void SBI_JavaScriptIcon::openJavaScriptSettings()
{
    JsOptions* dialog = new JsOptions(m_window);
    dialog->open();
}
