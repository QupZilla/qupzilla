/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2013  David Rosca <nowrep@gmail.com>
* Copyright (C) 2013  S. Razi Alavizadeh <s.r.alavizadeh@gmail.com>
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
#include "macmenureceiver.h"
#include "mainapplication.h"
#include "qupzilla.h"
#include "settings.h"

#include <QAction>
#include <QMenu>

#include <QDebug>

MacMenuReceiver::MacMenuReceiver(QObject* parent)
    : QObject(parent)
    , m_macMenuBar(0)
    , m_bookmarksMenuChanged(true)
    , m_menuBookmarksAction(0)
{
}

void MacMenuReceiver::setEnabledSelectedMenuActions(QMenu* menu, const QList<int> indexList)
{
    if (!menu || mApp->windowCount() == 0) {
        return;
    }

    // we mean all actions by empty list
    if (indexList.isEmpty()) {
        foreach(QAction * act, menu->actions()) {
            act->setEnabled(true);
        }
        return;
    }

    foreach(int index, indexList) {
        Q_ASSERT(index >= 0 && index < menu->actions().size());
        menu->actions().at(index)->setEnabled(true);
    }
}

void MacMenuReceiver::setDisabledSelectedMenuActions(QMenu* menu, const QList<int> indexList)
{
    if (!menu) {
        return;
    }

    // we mean all actions by empty list
    if (indexList.isEmpty()) {
        foreach(QAction * act, menu->actions()) {
            act->setDisabled(true);
        }
        return;
    }

    foreach(int index, indexList) {
        Q_ASSERT(index >= 0 && index < menu->actions().size());
        menu->actions().at(index)->setDisabled(true);
    }
}

bool MacMenuReceiver::callSlot(const char* member, bool makeIfNoWindow, QGenericArgument val0, QGenericArgument val1)
{
    //qDebug("MacMenuReceiver::callSlot: \'QupZilla::%s()\'", member);

    QupZilla* qzWindow = mApp->getWindow();
    if (!qzWindow) {
        if (!makeIfNoWindow) {
            return false;
        }
        else {
            qzWindow = mApp->makeNewWindow(Qz::BW_MacFirstWindow);
            mApp->processEvents();
        }
    }

    bool success = QMetaObject::invokeMethod(qzWindow, member, val0, val1);

    if (!success) {
        qCritical("Warning: try to invoke \'QupZilla::%s()\' with incorrect parameter(s) or no such slot.", member);
        // this should never happen, for now:
        // we just ignore it!
    }

    return true;
}

void MacMenuReceiver::goNext()
{
    callSlot("goNext");
}


void MacMenuReceiver::goBack()
{
    callSlot("goBack");
}

void MacMenuReceiver::goHome()
{
    callSlot("goHome", true);
}

void MacMenuReceiver::stop()
{
    callSlot("stop");
}

void MacMenuReceiver::reload()
{
    callSlot("reload");
}

void MacMenuReceiver::reloadByPassCache()
{
    callSlot("reloadByPassCache");
}

void MacMenuReceiver::aboutQupZilla()
{
    callSlot("aboutQupZilla", true);
}

void MacMenuReceiver::addTab()
{
    if (!callSlot("addTab")) {
        mApp->makeNewWindow(Qz::BW_MacFirstWindow);
    }
}

void MacMenuReceiver::savePageScreen()
{
    callSlot("savePageScreen");
}

void MacMenuReceiver::searchOnPage()
{
    callSlot("searchOnPage");
}

void MacMenuReceiver::showCookieManager()
{
    callSlot("showCookieManager");
}

void MacMenuReceiver::showHistoryManager()
{
    callSlot("showHistoryManager");
}

void MacMenuReceiver::showBookmarksManager()
{
    callSlot("showBookmarksManager");
}

void MacMenuReceiver::showRSSManager()
{
    callSlot("showRSSManager");
}

void MacMenuReceiver::showDownloadManager()
{
    callSlot("showDownloadManager");
}

void MacMenuReceiver::showMenubar()
{
    callSlot("showMenubar");
}

void MacMenuReceiver::showNavigationToolbar()
{
    callSlot("showNavigationToolbar");
}

void MacMenuReceiver::showStatusbar()
{
    callSlot("showStatusbar");
}

void MacMenuReceiver::showClearPrivateData()
{
    callSlot("showClearPrivateData");
}

void MacMenuReceiver::showPreferences()
{
    callSlot("showPreferences", true);
}

void MacMenuReceiver::showBookmarkImport()
{
    callSlot("showBookmarkImport");
}


void MacMenuReceiver::refreshHistory()
{
    callSlot("refreshHistory");
}

void MacMenuReceiver::bookmarkAllTabs()
{
    callSlot("bookmarkAllTabs");
}

void MacMenuReceiver::newWindow()
{
    if (!callSlot("newWindow")) {
        mApp->makeNewWindow(Qz::BW_MacFirstWindow);
    }
}


void MacMenuReceiver::openLocation()
{
    callSlot("openLocation", true);
}

void MacMenuReceiver::openFile()
{
    callSlot("openFile", true);
}

void MacMenuReceiver::savePage()
{
    callSlot("savePage");
}

void MacMenuReceiver::sendLink()
{
    callSlot("sendLink");
}

void MacMenuReceiver::webSearch()
{
    callSlot("webSearch");
}


void MacMenuReceiver::editUndo()
{
    callSlot("editUndo");
}

void MacMenuReceiver::editRedo()
{
    callSlot("editRedo");
}

void MacMenuReceiver::editCut()
{
    callSlot("editCut");
}

void MacMenuReceiver::editCopy()
{
    callSlot("editCopy");
}

void MacMenuReceiver::editPaste()
{
    callSlot("editPaste");
}

void MacMenuReceiver::editSelectAll()
{
    callSlot("editSelectAll");
}


void MacMenuReceiver::zoomIn()
{
    callSlot("zoomIn");
}

void MacMenuReceiver::zoomOut()
{
    callSlot("zoomOut");
}

void MacMenuReceiver::zoomReset()
{
    callSlot("zoomReset");
}

void MacMenuReceiver::fullScreen(bool make)
{
    callSlot("fullScreen", false, Q_ARG(bool, make));
}

void MacMenuReceiver::changeEncoding(QObject* obj)
{
    if (!obj) {
        obj = sender();
    }

    callSlot("changeEncoding", false, Q_ARG(QObject*, obj));
}

void MacMenuReceiver::triggerCaretBrowsing()
{
    callSlot("triggerCaretBrowsing");
}

void MacMenuReceiver::triggerTabsOnTop(bool enable)
{
    callSlot("triggerTabsOnTop", false, Q_ARG(bool, enable));
}

void MacMenuReceiver::closeWindow()
{
    callSlot("closeWindow");
}

void MacMenuReceiver::quitApp()
{
    if (!callSlot("quitApp")) {
        mApp->quitApplication();
    }
}

void MacMenuReceiver::printPage(QWebFrame* frame)
{
    callSlot("printPage", false, Q_ARG(QWebFrame*, frame));
}

void MacMenuReceiver::showBookmarksToolbar()
{
    callSlot("showBookmarksToolbar");
}

void MacMenuReceiver::showSource(QWebFrame* frame, const QString &selectedHtml)
{
    callSlot("showSource", false, Q_ARG(QWebFrame*, frame), Q_ARG(const QString &, selectedHtml));
}

void MacMenuReceiver::bookmarkPage()
{
    callSlot("bookmarkPage");
}

void MacMenuReceiver::loadFolderBookmarks(Menu* menu)
{
    callSlot("loadFolderBookmarks", false, Q_ARG(Menu*, menu));
}

void MacMenuReceiver::closeTab()
{
    callSlot("closeTab");
}

void MacMenuReceiver::restoreClosedTab(QObject* obj)
{
    if (!obj) {
        obj = sender();
    }

    callSlot("restoreClosedTab", false, Q_ARG(QObject*, obj));
}

void MacMenuReceiver::restoreAllClosedTabs()
{
    callSlot("restoreAllClosedTabs");
}

void MacMenuReceiver::clearClosedTabsList()
{
    callSlot("clearClosedTabsList");
}

void MacMenuReceiver::showPageInfo()
{
    callSlot("showPageInfo");
}

void MacMenuReceiver::showWebInspector(bool toggle)
{
    callSlot("showWebInspector", false, Q_ARG(bool, toggle));
}

void MacMenuReceiver::loadActionUrl(QObject* obj)
{
    if (!obj) {
        obj = sender();
    }

    callSlot("loadActionUrl", true, Q_ARG(QObject*, obj));
}

void MacMenuReceiver::loadActionUrlInNewTab(QObject* obj)
{
    if (!obj) {
        obj = sender();
    }

    if (!callSlot("loadActionUrlInNewTab", false, Q_ARG(QObject*, obj))) {
        callSlot("loadActionUrl", true, Q_ARG(QObject*, obj));
    }
}

void MacMenuReceiver::loadActionUrlInNewNotSelectedTab(QObject* obj)
{
    if (!obj) {
        obj = sender();
    }

    if (!callSlot("loadActionUrlInNewNotSelectedTab", false, Q_ARG(QObject*, obj))) {
        callSlot("loadActionUrl", true, Q_ARG(QObject*, obj));
    }
}

// about to show/hide slots
void MacMenuReceiver::aboutToShowFileMenu()
{
    QMenu* menu = qobject_cast<QMenu*>(sender());
    setEnabledSelectedMenuActions(menu);
    if (!callSlot("aboutToShowFileMenu")) {
        setDisabledSelectedMenuActions(menu, QList<int>()
                                       << 4 << 5 << 7 << 8 << 9 << 10 << 12);
    }
}

void MacMenuReceiver::aboutToHideFileMenu()
{
    callSlot("aboutToHideFileMenu");
}

void MacMenuReceiver::aboutToShowHistoryMenu()
{
    QMenu* menu = qobject_cast<QMenu*>(sender());
    // 2=Home, 3=Show all History, 7=Closed Tabs
    setEnabledSelectedMenuActions(menu, QList<int>() << 2 << 3 << 7);
    if (!callSlot("aboutToShowHistoryMenu")) {
        setDisabledSelectedMenuActions(menu, QList<int>()
                                       << 0 << 1 << 2 << 3 << 7);
    }
}

void MacMenuReceiver::aboutToHideHistoryMenu()
{
    callSlot("aboutToHideHistoryMenu");
}

void MacMenuReceiver::aboutToShowClosedTabsMenu()
{
    callSlot("aboutToShowClosedTabsMenu");
}

void MacMenuReceiver::aboutToShowBookmarksMenu()
{
    QMenu* menu = qobject_cast<QMenu*>(sender());
    setEnabledSelectedMenuActions(menu, QList<int>() << 0 << 1 << 2);
    if (!callSlot("aboutToShowBookmarksMenu")) {
        setDisabledSelectedMenuActions(menu, QList<int>() << 0 << 1 << 2);
    }
}

void MacMenuReceiver::aboutToShowViewMenu()
{
    QMenu* menu = qobject_cast<QMenu*>(sender());
    // 7,8,9=Zoom actions, 12=Character Encoding, 15=Fullscreen
    setEnabledSelectedMenuActions(menu, QList<int>()
                                  << 0 << 1 << 2 << 7 << 8 << 9 << 11 << 12 << 15);
    // for updating reload and stop actions
    if (mApp->getWindow()) {
        mApp->getWindow()->updateLoadingActions();
    }

    if (!callSlot("aboutToShowViewMenu")) {
        setDisabledSelectedMenuActions(menu);
    }
}

void MacMenuReceiver::aboutToHideViewMenu()
{
    callSlot("aboutToHideViewMenu");
}

void MacMenuReceiver::aboutToShowEditMenu()
{
    QMenu* menu = qobject_cast<QMenu*>(sender());
    // 8=Find
    setEnabledSelectedMenuActions(menu, QList<int>() << 8);
    if (!callSlot("aboutToShowEditMenu")) {
        setDisabledSelectedMenuActions(menu);
    }
}

void MacMenuReceiver::aboutToHideEditMenu()
{
    callSlot("aboutToHideEditMenu");
}

void MacMenuReceiver::aboutToShowToolsMenu()
{
    QMenu* menu = qobject_cast<QMenu*>(sender());
    // enable all
    setEnabledSelectedMenuActions(menu);
    if (!callSlot("aboutToShowToolsMenu")) {
        setDisabledSelectedMenuActions(menu, QList<int>()
                                       << 0 << 1 << 3 << 4 << 6 << 7 << 8);
    }
}

void MacMenuReceiver::aboutToHideToolsMenu()
{
    callSlot("aboutToHideToolsMenu");
}

void MacMenuReceiver::aboutToShowEncodingMenu()
{
    callSlot("aboutToShowEncodingMenu");
}

void MacMenuReceiver::aboutToShowHistoryRecentMenu()
{
    callSlot("aboutToShowHistoryRecentMenu");
}

void MacMenuReceiver::aboutToShowHistoryMostMenu()
{
    callSlot("aboutToShowHistoryMostMenu");
}
