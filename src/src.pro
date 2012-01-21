#-------------------------------------------------
#
# Project created by QtCreator 2010-12-18T14:53:41
#
#-------------------------------------------------

QT       += core gui webkit sql network script
TARGET = qupzilla
TEMPLATE = app

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR = ../build
RCC_DIR = ../build
UI_DIR = ../build

# Please read BUILD informations #
#DEFINES += NO_SYSTEM_DATAPATH
#DEFINES += USE_WEBGL
#DEFINES += KDE
#DEFINES += PORTABLE_BUILD
#win32:DEFINES += W7API

##It won't compile on windows with this define. Some bug in qtsingleapp / qvector template
!win32: !CONFIG(debug, debug|release): DEFINES += QT_NO_DEBUG_OUTPUT

TRANSLATIONS +=../translations/cs_CZ.ts\
               ../translations/sk_SK.ts\
               ../translations/de_DE.ts\
               ../translations/nl_NL.ts\
               ../translations/zh_CN.ts\
               ../translations/it_IT.ts\
               ../translations/pl_PL.ts\
               ../translations/es_ES.ts\
               ../translations/fr_FR.ts\
               ../translations/el_GR.ts\
               ../translations/ru_RU.ts\
               ../translations/pt_PT.ts\
               ../translations/sr_BA.ts\
               ../translations/sr_RS.ts\
               ../translations/sv_SE.ts\
               ../translations/empty.ts\

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
               opensearch\
               bookmarksimport\
               popupwindow\

SOURCES += main.cpp\
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
    tools/animatedwidget.cpp \
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
    network/networkproxyfactory.cpp \
    tools/closedtabsmanager.cpp \
    other/statusbarmessage.cpp \
    tools/buttonbox.cpp \
    tools/widget.cpp \
    3rdparty/squeezelabelv2.cpp \
    3rdparty/squeezelabelv1.cpp \
    tools/buttonwithmenu.cpp \
    navigation/locationbarsettings.cpp \
    other/browsinglibrary.cpp \
    3rdparty/stylehelper.cpp \
    3rdparty/fancytabwidget.cpp \
    history/webhistoryinterface.cpp \
    tools/toolbutton.cpp \
    navigation/navigationbar.cpp \
    navigation/reloadstopbutton.cpp \
    preferences/thememanager.cpp \
    network/qupzillaschemehandler.cpp \
    tools/globalfunctions.cpp \
    other/pagescreen.cpp \
    downloads/downloadfilehelper.cpp \
    tools/certificateinfowidget.cpp \
    webview/webinspectordockwidget.cpp \
    app/profileupdater.cpp \
    preferences/acceptlanguage.cpp \
    opensearch/opensearchreader.cpp \
    opensearch/opensearchengine.cpp \
    opensearch/opensearchenginedelegate.cpp \
    opensearch/searchenginesmanager.cpp \
    opensearch/searchenginesdialog.cpp \
    opensearch/editsearchengine.cpp \
    bookmarksimport/firefoximporter.cpp \
    bookmarksimport/chromeimporter.cpp \
    bookmarksimport/operaimporter.cpp \
    bookmarksimport/bookmarksimportdialog.cpp \
    tools/iconfetcher.cpp \
    tools/followredirectreply.cpp \
    webview/webhistorywrapper.cpp \
    tools/pagethumbnailer.cpp \
    plugins/speeddial.cpp \
    other/databasewriter.cpp \
    bookmarksimport/htmlimporter.cpp \
    tools/enhancedmenu.cpp \
    navigation/siteicon.cpp \
    navigation/goicon.cpp \
    rss/rssicon.cpp \
    navigation/downicon.cpp \
    network/cabundleupdater.cpp \
    app/settings.cpp \
    app/proxystyle.cpp \
    popupwindow/popupwebpage.cpp \
    webview/popupwebview.cpp \
    popupwindow/popupwindow.cpp \
    popupwindow/popuplocationbar.cpp \
    webview/tabbedwebview.cpp \
    webview/webview.cpp

HEADERS  += \
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
    tools/animatedwidget.h \
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
    network/networkproxyfactory.h \
    tools/closedtabsmanager.h \
    other/statusbarmessage.h \
    tools/buttonbox.h \
    tools/widget.h \
    3rdparty/squeezelabelv2.h \
    3rdparty/squeezelabelv1.h \
    tools/buttonwithmenu.h \
    navigation/locationbarsettings.h \
    other/browsinglibrary.h \
    3rdparty/stylehelper.h \
    3rdparty/fancytabwidget.h \
    history/webhistoryinterface.h \
    tools/toolbutton.h \
    navigation/navigationbar.h \
    navigation/reloadstopbutton.h \
    preferences/thememanager.h \
    network/qupzillaschemehandler.h \
    tools/globalfunctions.h \
    other/pagescreen.h \
    downloads/downloadfilehelper.h \
    tools/certificateinfowidget.h \
    webview/webinspectordockwidget.h \
    3rdparty/msvc2008.h \
    app/profileupdater.h \
    preferences/acceptlanguage.h \
    opensearch/opensearchreader.h \
    opensearch/opensearchengine.h \
    opensearch/opensearchenginedelegate.h \
    opensearch/searchenginesmanager.h \
    opensearch/searchenginesdialog.h \
    opensearch/editsearchengine.h \
    bookmarksimport/firefoximporter.h \
    bookmarksimport/chromeimporter.h \
    bookmarksimport/operaimporter.h \
    bookmarksimport/bookmarksimportdialog.h \
    tools/iconfetcher.h \
    tools/followredirectreply.h \
    webview/webhistorywrapper.h \
    tools/pagethumbnailer.h \
    plugins/speeddial.h \
    other/databasewriter.h \
    bookmarksimport/htmlimporter.h \
    tools/enhancedmenu.h \
    navigation/siteicon.h \
    navigation/goicon.h \
    rss/rssicon.h \
    navigation/downicon.h \
    network/cabundleupdater.h \
    app/settings.h \
    app/proxystyle.h \
    popupwindow/popupwebpage.h \
    webview/popupwebview.h \
    popupwindow/popupwindow.h \
    popupwindow/popuplocationbar.h \
    webview/tabbedwebview.h \
    webview/webview.h \
    app/qz_namespace.h

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
    desktopnotifications/desktopnotification.ui \
    webview/jsconfirm.ui \
    webview/jsalert.ui \
    webview/jsprompt.ui \
    other/browsinglibrary.ui \
    webview/searchtoolbar.ui \
    preferences/thememanager.ui \
    other/pagescreen.ui \
    tools/certificateinfowidget.ui \
    preferences/acceptlanguage.ui \
    preferences/addacceptlanguage.ui \
    opensearch/searchenginesdialog.ui \
    opensearch/editsearchengine.ui \
    bookmarksimport/bookmarksimportdialog.ui

RESOURCES += \
    data/icons.qrc \
    data/html.qrc \
    data/data.qrc

OTHER_FILES += \
    appicon.rc \
    appicon_os2.rc \
    Info.plist

include(3rdparty/qtsingleapplication.pri)

os2 {
    RC_FILE = appicon_os2.rc
}

win32 {
    RC_FILE = appicon.rc
    LIBS += User32.lib Ole32.lib Shell32.lib ShlWapi.lib Gdi32.lib ComCtl32.lib
}

mac {
    QMAKE_INFO_PLIST = Info.plist
    ICON = data/icons/exeicons/qupzilla.icns

    bundle_target.files += ../bin/locale
    bundle_target.files += ../bin/themes
    bundle_target.path = Resources

    QMAKE_BUNDLE_DATA += bundle_target
}

!mac:unix {
    QT += dbus

    d_prefix = $$(QUPZILLA_PREFIX)
    binary_folder = /usr/bin
    data_folder = /usr/share/qupzilla
    launcher_folder = /usr/share/applications
    icon_folder = /usr/share/pixmaps
    hicolor_folder = /usr/share/icons/hicolor

    !equals(d_prefix, "") {
        binary_folder = "$$d_prefix"bin
        data_folder = "$$d_prefix"share/qupzilla
        launcher_folder = "$$d_prefix"share/applications
        icon_folder = "$$d_prefix"share/pixmaps
        hicolor_folder = "$$d_prefix"share/icons/hicolor
    }

    DEFINES += USE_DATADIR=\\\"""$$data_folder/"\\\""

    target.path = $$binary_folder

    target1.files += ../bin/locale
    target1.files += ../bin/themes
    target1.path = $$data_folder

    target2.files = ../linux/applications/qupzilla.desktop
    target2.path = $$launcher_folder

    target3.files = ../linux/pixmaps/qupzilla.png
    target3.path = $$icon_folder

    ico16.files = ../linux/hicolor/16x16/qupzilla.png
    ico16.path = $$hicolor_folder/16x16/apps

    ico32.files = ../linux/hicolor/32x32/qupzilla.png
    ico32.path = $$hicolor_folder/32x32/apps

    ico48.files = ../linux/hicolor/48x48/qupzilla.png
    ico48.path = $$hicolor_folder/48x48/apps

    ico64.files = ../linux/hicolor/64x64/qupzilla.png
    ico64.path = $$hicolor_folder/64x64/apps

    ico128.files = ../linux/hicolor/128x128/qupzilla.png
    ico128.path = $$hicolor_folder/128x128/apps

    ico256.files = ../linux/hicolor/256x256/qupzilla.png
    ico256.path = $$hicolor_folder/256x256/apps

    INSTALLS += target target1 target2 target3
    INSTALLS += ico16 ico32 ico48 ico64 ico128 ico256

    #Git revision
    rev = $$system(sh ../scripts/getrevision.sh)
    !equals(rev, ""): DEFINES += GIT_REVISION=\\\"""$$rev"\\\""
}

d_no_system_datapath = $$(NO_SYSTEM_DATAPATH)
d_use_webgl = $$(USE_WEBGL)
d_w7api = $$(W7API)
d_kde = $$(KDE)
d_portable = $$(PORTABLE_BUILD)

equals(d_no_system_datapath, "true") { DEFINES += NO_SYSTEM_DATAPATH }
equals(d_use_webgl, "true") { DEFINES += USE_WEBGL }
equals(d_w7api, "true") { DEFINES += W7API }
equals(d_kde, "true") { DEFINES += KDE }
equals(d_portable, "true") { DEFINES += PORTABLE_BUILD }

message(Using following defines)
message($$DEFINES)


