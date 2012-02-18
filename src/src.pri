INCLUDEPATH += $$PWD/3rdparty\
               $$PWD/app\
               $$PWD/autofill\
               $$PWD/bookmarks\
               $$PWD/cookies\
               $$PWD/downloads\
               $$PWD/history\
               $$PWD/navigation\
               $$PWD/network\
               $$PWD/other\
               $$PWD/preferences\
               $$PWD/rss\
               $$PWD/tools\
               $$PWD/utils\
               $$PWD/webview\
               $$PWD/plugins\
               $$PWD/sidebar\
               $$PWD/data\
               $$PWD/adblock\
               $$PWD/desktopnotifications\
               $$PWD/opensearch\
               $$PWD/bookmarksimport\
               $$PWD/popupwindow\

SOURCES += $$PWD/main.cpp\
    $$PWD/3rdparty/qtwin.cpp \
    $$PWD/3rdparty/lineedit.cpp \
    $$PWD/app/qupzilla.cpp \
    $$PWD/app/mainapplication.cpp \
    $$PWD/app/autosaver.cpp \
    $$PWD/autofill/autofillmodel.cpp \
    $$PWD/preferences/autofillmanager.cpp \
    $$PWD/bookmarks/bookmarkstoolbar.cpp \
    $$PWD/bookmarks/bookmarksmanager.cpp \
    $$PWD/cookies/cookiemanager.cpp \
    $$PWD/cookies/cookiejar.cpp \
    $$PWD/downloads/downloadmanager.cpp \
    $$PWD/history/historymodel.cpp \
    $$PWD/history/historymanager.cpp \
    $$PWD/navigation/websearchbar.cpp \
    $$PWD/navigation/locationcompleter.cpp \
    $$PWD/navigation/locationbar.cpp \
    $$PWD/network/networkmanagerproxy.cpp \
    $$PWD/network/networkmanager.cpp \
    $$PWD/other/updater.cpp \
    $$PWD/other/sourceviewer.cpp \
    $$PWD/preferences/preferences.cpp \
    $$PWD/rss/rssmanager.cpp \
    $$PWD/other/clearprivatedata.cpp \
    $$PWD/webview/webpage.cpp \
    $$PWD/webview/tabwidget.cpp \
    $$PWD/webview/tabbar.cpp \
    $$PWD/webview/siteinfo.cpp \
    $$PWD/webview/searchtoolbar.cpp \
    $$PWD/app/commandlineoptions.cpp \
    $$PWD/other/aboutdialog.cpp \
    $$PWD/plugins/plugins.cpp \
    $$PWD/preferences/pluginslist.cpp \
    $$PWD/plugins/pluginproxy.cpp \
    $$PWD/tools/clickablelabel.cpp \
    $$PWD/downloads/downloadoptionsdialog.cpp \
    $$PWD/tools/treewidget.cpp \
    $$PWD/bookmarks/bookmarkswidget.cpp \
    $$PWD/tools/frame.cpp \
    $$PWD/bookmarks/bookmarksmodel.cpp \
    $$PWD/sidebar/sidebar.cpp \
    $$PWD/webview/siteinfowidget.cpp \
    $$PWD/plugins/clicktoflash.cpp \
    $$PWD/plugins/webpluginfactory.cpp \
    $$PWD/downloads/downloaditem.cpp \
    $$PWD/3rdparty/ecwin7.cpp \
    $$PWD/webview/webtab.cpp \
    $$PWD/rss/rsswidget.cpp \
    $$PWD/autofill/autofillnotification.cpp \
    $$PWD/rss/rssnotification.cpp \
    $$PWD/navigation/locationpopup.cpp \
    $$PWD/preferences/sslmanager.cpp \
    $$PWD/tools/animatedwidget.cpp \
    $$PWD/tools/htmlhighlighter.cpp \
    $$PWD/other/sourceviewersearch.cpp \
    $$PWD/adblock/adblocksubscription.cpp \
    $$PWD/adblock/adblockrule.cpp \
    $$PWD/adblock/adblockpage.cpp \
    $$PWD/adblock/adblocknetwork.cpp \
    $$PWD/adblock/adblockmanager.cpp \
    $$PWD/adblock/adblockdialog.cpp \
    $$PWD/adblock/adblockblockednetworkreply.cpp \
    $$PWD/adblock/adblockicon.cpp \
    $$PWD/tools/docktitlebarwidget.cpp \
    $$PWD/sidebar/bookmarkssidebar.cpp \
    $$PWD/bookmarks/bookmarkicon.cpp \
    $$PWD/sidebar/historysidebar.cpp \
    $$PWD/desktopnotifications/desktopnotification.cpp \
    $$PWD/desktopnotifications/desktopnotificationsfactory.cpp \
    $$PWD/tools/progressbar.cpp \
    $$PWD/tools/iconprovider.cpp \
    $$PWD/network/networkproxyfactory.cpp \
    $$PWD/tools/closedtabsmanager.cpp \
    $$PWD/other/statusbarmessage.cpp \
    $$PWD/tools/buttonbox.cpp \
    $$PWD/tools/widget.cpp \
    $$PWD/3rdparty/squeezelabelv2.cpp \
    $$PWD/3rdparty/squeezelabelv1.cpp \
    $$PWD/tools/buttonwithmenu.cpp \
    $$PWD/navigation/locationbarsettings.cpp \
    $$PWD/other/browsinglibrary.cpp \
    $$PWD/3rdparty/stylehelper.cpp \
    $$PWD/3rdparty/fancytabwidget.cpp \
    $$PWD/history/webhistoryinterface.cpp \
    $$PWD/tools/toolbutton.cpp \
    $$PWD/navigation/navigationbar.cpp \
    $$PWD/navigation/reloadstopbutton.cpp \
    $$PWD/preferences/thememanager.cpp \
    $$PWD/network/qupzillaschemehandler.cpp \
    $$PWD/tools/globalfunctions.cpp \
    $$PWD/other/pagescreen.cpp \
    $$PWD/downloads/downloadfilehelper.cpp \
    $$PWD/tools/certificateinfowidget.cpp \
    $$PWD/webview/webinspectordockwidget.cpp \
    $$PWD/app/profileupdater.cpp \
    $$PWD/preferences/acceptlanguage.cpp \
    $$PWD/opensearch/opensearchreader.cpp \
    $$PWD/opensearch/opensearchengine.cpp \
    $$PWD/opensearch/opensearchenginedelegate.cpp \
    $$PWD/opensearch/searchenginesmanager.cpp \
    $$PWD/opensearch/searchenginesdialog.cpp \
    $$PWD/opensearch/editsearchengine.cpp \
    $$PWD/bookmarksimport/firefoximporter.cpp \
    $$PWD/bookmarksimport/chromeimporter.cpp \
    $$PWD/bookmarksimport/operaimporter.cpp \
    $$PWD/bookmarksimport/bookmarksimportdialog.cpp \
    $$PWD/tools/iconfetcher.cpp \
    $$PWD/tools/followredirectreply.cpp \
    $$PWD/webview/webhistorywrapper.cpp \
    $$PWD/tools/pagethumbnailer.cpp \
    $$PWD/plugins/speeddial.cpp \
    $$PWD/other/databasewriter.cpp \
    $$PWD/bookmarksimport/htmlimporter.cpp \
    $$PWD/tools/enhancedmenu.cpp \
    $$PWD/navigation/siteicon.cpp \
    $$PWD/navigation/goicon.cpp \
    $$PWD/rss/rssicon.cpp \
    $$PWD/navigation/downicon.cpp \
    $$PWD/network/cabundleupdater.cpp \
    $$PWD/app/settings.cpp \
    $$PWD/app/proxystyle.cpp \
    $$PWD/popupwindow/popupwebpage.cpp \
    $$PWD/webview/popupwebview.cpp \
    $$PWD/popupwindow/popupwindow.cpp \
    $$PWD/popupwindow/popuplocationbar.cpp \
    $$PWD/webview/tabbedwebview.cpp \
    $$PWD/webview/webview.cpp \
    $$PWD/webview/webviewsettings.cpp

HEADERS  += \
    $$PWD/3rdparty/qtwin.h \
    $$PWD/3rdparty/lineedit.h \
    $$PWD/app/qupzilla.h \
    $$PWD/app/mainapplication.h \
    $$PWD/app/autosaver.h \
    $$PWD/autofill/autofillmodel.h \
    $$PWD/preferences/autofillmanager.h \
    $$PWD/bookmarks/bookmarkstoolbar.h \
    $$PWD/bookmarks/bookmarksmanager.h \
    $$PWD/cookies/cookiemanager.h \
    $$PWD/cookies/cookiejar.h \
    $$PWD/downloads/downloadmanager.h \
    $$PWD/history/historymodel.h \
    $$PWD/history/historymanager.h \
    $$PWD/navigation/websearchbar.h \
    $$PWD/navigation/locationcompleter.h \
    $$PWD/navigation/locationbar.h \
    $$PWD/network/networkmanagerproxy.h \
    $$PWD/network/networkmanager.h \
    $$PWD/other/updater.h \
    $$PWD/other/sourceviewer.h \
    $$PWD/preferences/preferences.h \
    $$PWD/rss/rssmanager.h \
    $$PWD/other/clearprivatedata.h \
    $$PWD/webview/webpage.h \
    $$PWD/webview/tabwidget.h \
    $$PWD/webview/tabbar.h \
    $$PWD/webview/siteinfo.h \
    $$PWD/webview/searchtoolbar.h \
    $$PWD/app/commandlineoptions.h \
    $$PWD/other/aboutdialog.h \
    $$PWD/plugins/plugininterface.h \
    $$PWD/plugins/plugins.h \
    $$PWD/preferences/pluginslist.h \
    $$PWD/plugins/pluginproxy.h \
    $$PWD/tools/clickablelabel.h \
    $$PWD/downloads/downloadoptionsdialog.h \
    $$PWD/tools/treewidget.h \
    $$PWD/bookmarks/bookmarkswidget.h \
    $$PWD/tools/frame.h \
    $$PWD/bookmarks/bookmarksmodel.h \
    $$PWD/sidebar/sidebar.h \
    $$PWD/webview/siteinfowidget.h \
    $$PWD/plugins/clicktoflash.h \
    $$PWD/plugins/webpluginfactory.h \
    $$PWD/downloads/downloaditem.h \
    $$PWD/3rdparty/ecwin7.h \
    $$PWD/webview/webtab.h \
    $$PWD/rss/rsswidget.h \
    $$PWD/autofill/autofillnotification.h \
    $$PWD/rss/rssnotification.h \
    $$PWD/navigation/locationpopup.h \
    $$PWD/preferences/sslmanager.h \
    $$PWD/tools/animatedwidget.h \
    $$PWD/tools/htmlhighlighter.h \
    $$PWD/other/sourceviewersearch.h \
    $$PWD/adblock/adblocksubscription.h \
    $$PWD/adblock/adblockrule.h \
    $$PWD/adblock/adblockpage.h \
    $$PWD/adblock/adblocknetwork.h \
    $$PWD/adblock/adblockmanager.h \
    $$PWD/adblock/adblockdialog.h \
    $$PWD/adblock/adblockblockednetworkreply.h \
    $$PWD/adblock/adblockicon.h \
    $$PWD/tools/docktitlebarwidget.h \
    $$PWD/sidebar/bookmarkssidebar.h \
    $$PWD/bookmarks/bookmarkicon.h \
    $$PWD/sidebar/historysidebar.h \
    $$PWD/desktopnotifications/desktopnotification.h \
    $$PWD/desktopnotifications/desktopnotificationsfactory.h \
    $$PWD/tools/progressbar.h \
    $$PWD/tools/iconprovider.h \
    $$PWD/network/networkproxyfactory.h \
    $$PWD/tools/closedtabsmanager.h \
    $$PWD/other/statusbarmessage.h \
    $$PWD/tools/buttonbox.h \
    $$PWD/tools/widget.h \
    $$PWD/3rdparty/squeezelabelv2.h \
    $$PWD/3rdparty/squeezelabelv1.h \
    $$PWD/tools/buttonwithmenu.h \
    $$PWD/navigation/locationbarsettings.h \
    $$PWD/other/browsinglibrary.h \
    $$PWD/3rdparty/stylehelper.h \
    $$PWD/3rdparty/fancytabwidget.h \
    $$PWD/history/webhistoryinterface.h \
    $$PWD/tools/toolbutton.h \
    $$PWD/navigation/navigationbar.h \
    $$PWD/navigation/reloadstopbutton.h \
    $$PWD/preferences/thememanager.h \
    $$PWD/network/qupzillaschemehandler.h \
    $$PWD/tools/globalfunctions.h \
    $$PWD/other/pagescreen.h \
    $$PWD/downloads/downloadfilehelper.h \
    $$PWD/tools/certificateinfowidget.h \
    $$PWD/webview/webinspectordockwidget.h \
    $$PWD/3rdparty/msvc2008.h \
    $$PWD/app/profileupdater.h \
    $$PWD/preferences/acceptlanguage.h \
    $$PWD/opensearch/opensearchreader.h \
    $$PWD/opensearch/opensearchengine.h \
    $$PWD/opensearch/opensearchenginedelegate.h \
    $$PWD/opensearch/searchenginesmanager.h \
    $$PWD/opensearch/searchenginesdialog.h \
    $$PWD/opensearch/editsearchengine.h \
    $$PWD/bookmarksimport/firefoximporter.h \
    $$PWD/bookmarksimport/chromeimporter.h \
    $$PWD/bookmarksimport/operaimporter.h \
    $$PWD/bookmarksimport/bookmarksimportdialog.h \
    $$PWD/tools/iconfetcher.h \
    $$PWD/tools/followredirectreply.h \
    $$PWD/webview/webhistorywrapper.h \
    $$PWD/tools/pagethumbnailer.h \
    $$PWD/plugins/speeddial.h \
    $$PWD/other/databasewriter.h \
    $$PWD/bookmarksimport/htmlimporter.h \
    $$PWD/tools/enhancedmenu.h \
    $$PWD/navigation/siteicon.h \
    $$PWD/navigation/goicon.h \
    $$PWD/rss/rssicon.h \
    $$PWD/navigation/downicon.h \
    $$PWD/network/cabundleupdater.h \
    $$PWD/app/settings.h \
    $$PWD/app/proxystyle.h \
    $$PWD/popupwindow/popupwebpage.h \
    $$PWD/webview/popupwebview.h \
    $$PWD/popupwindow/popupwindow.h \
    $$PWD/popupwindow/popuplocationbar.h \
    $$PWD/webview/tabbedwebview.h \
    $$PWD/webview/webview.h \
    $$PWD/app/qz_namespace.h \
    $$PWD/webview/webviewsettings.h

FORMS    += \
    $$PWD/preferences/autofillmanager.ui \
    $$PWD/bookmarks/bookmarksmanager.ui \
    $$PWD/cookies/cookiemanager.ui \
    $$PWD/history/historymanager.ui \
    $$PWD/preferences/preferences.ui \
    $$PWD/rss/rssmanager.ui \
    $$PWD/webview/siteinfo.ui \
    $$PWD/other/aboutdialog.ui \
    $$PWD/preferences/pluginslist.ui \
    $$PWD/downloads/downloadoptionsdialog.ui \
    $$PWD/bookmarks/bookmarkswidget.ui \
    $$PWD/webview/siteinfowidget.ui \
    $$PWD/downloads/downloaditem.ui \
    $$PWD/downloads/downloadmanager.ui \
    $$PWD/rss/rsswidget.ui \
    $$PWD/autofill/autofillnotification.ui \
    $$PWD/rss/rssnotification.ui \
    $$PWD/preferences/sslmanager.ui \
    $$PWD/other/clearprivatedata.ui \
    $$PWD/other/sourceviewersearch.ui \
    $$PWD/other/closedialog.ui \
    $$PWD/adblock/adblockdialog.ui \
    $$PWD/tools/docktitlebarwidget.ui \
    $$PWD/sidebar/bookmarkssidebar.ui \
    $$PWD/sidebar/historysidebar.ui \
    $$PWD/desktopnotifications/desktopnotification.ui \
    $$PWD/webview/jsconfirm.ui \
    $$PWD/webview/jsalert.ui \
    $$PWD/webview/jsprompt.ui \
    $$PWD/other/browsinglibrary.ui \
    $$PWD/webview/searchtoolbar.ui \
    $$PWD/preferences/thememanager.ui \
    $$PWD/other/pagescreen.ui \
    $$PWD/tools/certificateinfowidget.ui \
    $$PWD/preferences/acceptlanguage.ui \
    $$PWD/preferences/addacceptlanguage.ui \
    $$PWD/opensearch/searchenginesdialog.ui \
    $$PWD/opensearch/editsearchengine.ui \
    $$PWD/bookmarksimport/bookmarksimportdialog.ui

RESOURCES += \
    $$PWD/data/icons.qrc \
    $$PWD/data/html.qrc \
    $$PWD/data/data.qrc

OTHER_FILES += \
    $$PWD/appicon.rc \
    $$PWD/appicon_os2.rc \
    $$PWD/Info.plist

os2:RC_FILE = $$PWD/appicon_os2.rc
win32:RC_FILE = $$PWD/appicon.rc
