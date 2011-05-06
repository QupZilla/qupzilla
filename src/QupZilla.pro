#-------------------------------------------------
#
# Project created by QtCreator 2010-12-18T14:53:41
#
#-------------------------------------------------

QT       += core gui webkit sql network
TARGET = qupzilla
TEMPLATE = app
VERSION = 0.90.9
TRANSLATIONS +=cs_CZ.ts\
               sk_SK.ts

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR = ../build
RCC_DIR = ../build
UI_DIR = ../build

##It won't compile on windows with this define
##Some bug in qtsingleapp / qvector template
!win32: !CONFIG(debug, debug|release): DEFINES += QT_NO_DEBUG_OUTPUT

INCLUDEPATH += 3rdparty\
               app\
               autofill\
               bookmarks\
               cookies\
               downloads\
               history\
               navigation\
               network\
               other\
               preferences\
               rss\
               tools\
               utils\
               webview\
               plugins\
               sidebar\
               data\
               adblock\
               desktopnotifications\

SOURCES += main.cpp\
    3rdparty/squeezelabel.cpp \
    3rdparty/qtwin.cpp \
    3rdparty/lineedit.cpp \
    app/qupzilla.cpp \
    app/mainapplication.cpp \
    app/autosaver.cpp \
    autofill/autofillmodel.cpp \
    preferences/autofillmanager.cpp \
    bookmarks/bookmarkstoolbar.cpp \
    bookmarks/bookmarksmanager.cpp \
    cookies/cookiemanager.cpp \
    cookies/cookiejar.cpp \
    downloads/downloadmanager.cpp \
    history/historymodel.cpp \
    history/historymanager.cpp \
    navigation/websearchbar.cpp \
    navigation/locationcompleter.cpp \
    navigation/locationbar.cpp \
    network/networkmanagerproxy.cpp \
    network/networkmanager.cpp \
    other/updater.cpp \
    other/sourceviewer.cpp \
    preferences/preferences.cpp \
    rss/rssmanager.cpp \
    other/clearprivatedata.cpp \
    webview/webview.cpp \
    webview/webpage.cpp \
    webview/tabwidget.cpp \
    webview/tabbar.cpp \
    webview/siteinfo.cpp \
    webview/searchtoolbar.cpp \
    app/commandlineoptions.cpp \
    other/aboutdialog.cpp \
    plugins/plugins.cpp \
    preferences/pluginslist.cpp \
    plugins/pluginproxy.cpp \
    app/appui.cpp \
    tools/clickablelabel.cpp \
    downloads/downloadoptionsdialog.cpp \
    tools/treewidget.cpp \
    bookmarks/bookmarkswidget.cpp \
    tools/frame.cpp \
    bookmarks/bookmarksmodel.cpp \
    sidebar/sidebar.cpp \
    webview/siteinfowidget.cpp \
    plugins/clicktoflash.cpp \
    plugins/webpluginfactory.cpp \
    downloads/downloaditem.cpp \
    3rdparty/ecwin7.cpp \
    webview/webtab.cpp \
    rss/rsswidget.cpp \
    autofill/autofillnotification.cpp \
    rss/rssnotification.cpp \
    navigation/locationpopup.cpp \
    preferences/sslmanager.cpp \
    tools/notification.cpp \
    tools/htmlhighlighter.cpp \
    other/sourceviewersearch.cpp \
    adblock/adblocksubscription.cpp \
    adblock/adblockrule.cpp \
    adblock/adblockpage.cpp \
    adblock/adblocknetwork.cpp \
    adblock/adblockmanager.cpp \
    adblock/adblockdialog.cpp \
    adblock/adblockblockednetworkreply.cpp \
    adblock/adblockicon.cpp \
    tools/docktitlebarwidget.cpp \
    sidebar/bookmarkssidebar.cpp \
    bookmarks/bookmarkicon.cpp \
    sidebar/historysidebar.cpp \
    desktopnotifications/desktopnotification.cpp \
    desktopnotifications/desktopnotificationsfactory.cpp \
    tools/progressbar.cpp \
    tools/iconprovider.cpp \
    network/networkproxyfactory.cpp

HEADERS  += 3rdparty/squeezelabel.h \
    3rdparty/qtwin.h \
    3rdparty/lineedit.h \
    app/qupzilla.h \
    app/mainapplication.h \
    app/autosaver.h \
    autofill/autofillmodel.h \
    preferences/autofillmanager.h \
    bookmarks/bookmarkstoolbar.h \
    bookmarks/bookmarksmanager.h \
    cookies/cookiemanager.h \
    cookies/cookiejar.h \
    downloads/downloadmanager.h \
    history/historymodel.h \
    history/historymanager.h \
    navigation/websearchbar.h \
    navigation/locationcompleter.h \
    navigation/locationbar.h \
    network/networkmanagerproxy.h \
    network/networkmanager.h \
    other/updater.h \
    other/sourceviewer.h \
    preferences/preferences.h \
    rss/rssmanager.h \
    other/clearprivatedata.h \
    webview/webview.h \
    webview/webpage.h \
    webview/tabwidget.h \
    webview/tabbar.h \
    webview/siteinfo.h \
    webview/searchtoolbar.h \
    app/commandlineoptions.h \
    other/aboutdialog.h \
    plugins/plugininterface.h \
    plugins/plugins.h \
    preferences/pluginslist.h \
    plugins/pluginproxy.h \
    tools/clickablelabel.h \
    downloads/downloadoptionsdialog.h \
    tools/treewidget.h \
    bookmarks/bookmarkswidget.h \
    tools/frame.h \
    bookmarks/bookmarksmodel.h \
    sidebar/sidebar.h \
    webview/siteinfowidget.h \
    plugins/clicktoflash.h \
    plugins/webpluginfactory.h \
    downloads/downloaditem.h \
    3rdparty/ecwin7.h \
    webview/webtab.h \
    rss/rsswidget.h \
    autofill/autofillnotification.h \
    rss/rssnotification.h \
    navigation/locationpopup.h \
    preferences/sslmanager.h \
    tools/notification.h \
    tools/htmlhighlighter.h \
    other/sourceviewersearch.h \
    adblock/adblocksubscription.h \
    adblock/adblockrule.h \
    adblock/adblockpage.h \
    adblock/adblocknetwork.h \
    adblock/adblockmanager.h \
    adblock/adblockdialog.h \
    adblock/adblockblockednetworkreply.h \
    adblock/adblockicon.h \
    tools/docktitlebarwidget.h \
    sidebar/bookmarkssidebar.h \
    bookmarks/bookmarkicon.h \
    sidebar/historysidebar.h \
    desktopnotifications/desktopnotification.h \
    desktopnotifications/desktopnotificationsfactory.h \
    tools/progressbar.h \
    tools/iconprovider.h \
    network/networkproxyfactory.h

FORMS    += \
    preferences/autofillmanager.ui \
    bookmarks/bookmarksmanager.ui \
    cookies/cookiemanager.ui \
    history/historymanager.ui \
    preferences/preferences.ui \
    rss/rssmanager.ui \
    webview/siteinfo.ui \
    other/aboutdialog.ui \
    preferences/pluginslist.ui \
    downloads/downloadoptionsdialog.ui \
    bookmarks/bookmarkswidget.ui \
    webview/siteinfowidget.ui \
    downloads/downloaditem.ui \
    downloads/downloadmanager.ui \
    rss/rsswidget.ui \
    autofill/autofillnotification.ui \
    rss/rssnotification.ui \
    preferences/sslmanager.ui \
    other/clearprivatedata.ui \
    other/sourceviewersearch.ui \
    other/closedialog.ui \
    adblock/adblockdialog.ui \
    tools/docktitlebarwidget.ui \
    sidebar/bookmarkssidebar.ui \
    sidebar/historysidebar.ui \
    desktopnotifications/desktopnotification.ui

RESOURCES += \
    data/icons.qrc \
    data/html.qrc

OTHER_FILES += \
    appicon.rc

include(3rdparty/qtsingleapplication.pri)

unix:QT += dbus
win32:DEFINES += W7API
win32:RC_FILE = appicon.rc
win32:LIBS += User32.lib Ole32.lib Shell32.lib ShlWapi.lib Gdi32.lib ComCtl32.lib DwmApi.lib
