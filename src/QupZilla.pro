#-------------------------------------------------
#
# Project created by QtCreator 2010-12-18T14:53:41
#
#-------------------------------------------------

DESTDIR = ../bin

QT       += core gui webkit sql network

TARGET = qupzilla

TEMPLATE = app

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
    autofill/autofillwidget.cpp \
    webview/webtab.cpp

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
    autofill/autofillwidget.h \
    webview/webtab.h

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
    autofill/autofillwidget.ui

RESOURCES += \
    data/icons.qrc \
    data/html.qrc

OTHER_FILES += \
    appicon.rc \

TRANSLATIONS +=cs_CZ.ts

include(3rdparty/qtsingleapplication.pri)

win32:RC_FILE = appicon.rc
win32:LIBS += libole32
