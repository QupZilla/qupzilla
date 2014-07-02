isEqual(QT_MAJOR_VERSION, 5) {
    QT += webkitwidgets network widgets printsupport sql script gui-private
} else {
    QT += core gui webkit sql network script concurrent
}

TARGET = QupZilla
TEMPLATE = lib

DEFINES *= QUPZILLA_SHAREDLIBRARY

include(../defines.pri)
include(../../translations/translations.pri)
include(3rdparty/qtsingleapplication/qtsingleapplication.pri)
include(plugins/qtwebkit/qtwebkit-plugins.pri)

CONFIG(debug, debug|release): include(../../tests/modeltest/modeltest.pri)

unix:!contains(DEFINES, "DISABLE_DBUS") QT += dbus

INCLUDEPATH += 3rdparty \
               adblock \
               app \
               autofill \
               bookmarks \
               cookies \
               downloads \
               history \
               navigation \
               network \
               notifications \
               opensearch \
               other \
               plugins \
               popupwindow \
               preferences \
               rss \
               session \
               sidebar \
               tabwidget \
               tools \
               webkit \
               webtab \

DEPENDPATH += $$INCLUDEPATH \
              data \

SOURCES += \
    3rdparty/ecwin7.cpp \
    3rdparty/fancytabwidget.cpp \
    3rdparty/lineedit.cpp \
    3rdparty/processinfo.cpp \
    3rdparty/qtwin.cpp \
    3rdparty/squeezelabelv1.cpp \
    3rdparty/squeezelabelv2.cpp \
    3rdparty/stylehelper.cpp \
    adblock/adblockaddsubscriptiondialog.cpp \
    adblock/adblockblockednetworkreply.cpp \
    adblock/adblockdialog.cpp \
    adblock/adblockicon.cpp \
    adblock/adblockmanager.cpp \
    adblock/adblockrule.cpp \
    adblock/adblocksearchtree.cpp \
    adblock/adblocksubscription.cpp \
    adblock/adblocktreewidget.cpp \
    app/autosaver.cpp \
    app/browserwindow.cpp \
    app/commandlineoptions.cpp \
    app/datapaths.cpp \
    app/mainapplication.cpp \
    app/mainmenu.cpp \
    app/profilemanager.cpp \
    app/proxystyle.cpp \
    app/qzcommon.cpp \
    app/settings.cpp \
    autofill/autofill.cpp \
    autofill/autofillicon.cpp \
    autofill/autofillnotification.cpp \
    autofill/autofillwidget.cpp \
    autofill/pageformcompleter.cpp \
    autofill/passwordbackends/databaseencryptedpasswordbackend.cpp \
    autofill/passwordbackends/databasepasswordbackend.cpp \
    autofill/passwordbackends/passwordbackend.cpp \
    autofill/passwordmanager.cpp \
    bookmarks/bookmarkitem.cpp \
    bookmarks/bookmarks.cpp \
    bookmarks/bookmarksexport/bookmarksexportdialog.cpp \
    bookmarks/bookmarksexport/bookmarksexporter.cpp \
    bookmarks/bookmarksexport/htmlexporter.cpp \
    bookmarks/bookmarksicon.cpp \
    bookmarks/bookmarksimport/bookmarksimportdialog.cpp \
    bookmarks/bookmarksimport/bookmarksimporter.cpp \
    bookmarks/bookmarksimport/firefoximporter.cpp \
    bookmarks/bookmarksimport/htmlimporter.cpp \
    bookmarks/bookmarksimport/chromeimporter.cpp \
    bookmarks/bookmarksimport/ieimporter.cpp \
    bookmarks/bookmarksimport/operaimporter.cpp \
    bookmarks/bookmarksitemdelegate.cpp \
    bookmarks/bookmarksmanager.cpp \
    bookmarks/bookmarksmenu.cpp \
    bookmarks/bookmarksmodel.cpp \
    bookmarks/bookmarkstoolbarbutton.cpp \
    bookmarks/bookmarkstoolbar.cpp \
    bookmarks/bookmarkstools.cpp \
    bookmarks/bookmarkstreeview.cpp \
    bookmarks/bookmarkswidget.cpp \
    cookies/cookiejar.cpp \
    cookies/cookiemanager.cpp \
    downloads/downloadfilehelper.cpp \
    downloads/downloaditem.cpp \
    downloads/downloadmanager.cpp \
    downloads/downloadoptionsdialog.cpp \
    history/history.cpp \
    history/historyitem.cpp \
    history/historymanager.cpp \
    history/historymenu.cpp \
    history/historymodel.cpp \
    history/historytreeview.cpp \
    history/webhistoryinterface.cpp \
    navigation/completer/locationcompleter.cpp \
    navigation/completer/locationcompleterdelegate.cpp \
    navigation/completer/locationcompletermodel.cpp \
    navigation/completer/locationcompleterrefreshjob.cpp \
    navigation/completer/locationcompleterview.cpp \
    navigation/downicon.cpp \
    navigation/goicon.cpp \
    navigation/locationbar.cpp \
    navigation/locationbarpopup.cpp \
    navigation/navigationbar.cpp \
    navigation/navigationcontainer.cpp \
    navigation/reloadstopbutton.cpp \
    navigation/siteicon.cpp \
    navigation/websearchbar.cpp \
    network/cabundleupdater.cpp \
    network/networkmanager.cpp \
    network/networkmanagerproxy.cpp \
    network/networkproxyfactory.cpp \
    network/pac/pacmanager.cpp \
    network/pac/proxyautoconfig.cpp \
    network/schemehandlers/adblockschemehandler.cpp \
    network/schemehandlers/fileschemehandler.cpp \
    network/schemehandlers/ftpschemehandler.cpp \
    network/schemehandlers/qupzillaschemehandler.cpp \
    network/sslerrordialog.cpp \
    notifications/desktopnotification.cpp \
    notifications/desktopnotificationsfactory.cpp \
    opensearch/editsearchengine.cpp \
    opensearch/opensearchengine.cpp \
    opensearch/opensearchenginedelegate.cpp \
    opensearch/opensearchreader.cpp \
    opensearch/searchenginesdialog.cpp \
    opensearch/searchenginesmanager.cpp \
    other/aboutdialog.cpp \
    other/browsinglibrary.cpp \
    other/clearprivatedata.cpp \
    other/checkboxdialog.cpp \
    other/iconchooser.cpp \
    other/licenseviewer.cpp \
    other/pagescreen.cpp \
    other/qzsettings.cpp \
    other/siteinfo.cpp \
    other/siteinfowidget.cpp \
    other/sourceviewer.cpp \
    other/sourceviewersearch.cpp \
    other/statusbarmessage.cpp \
    other/updater.cpp \
    other/useragentmanager.cpp \
    plugins/clicktoflash.cpp \
    plugins/pluginproxy.cpp \
    plugins/plugins.cpp \
    plugins/speeddial.cpp \
    popupwindow/popuplocationbar.cpp \
    popupwindow/popupstatusbarmessage.cpp \
    popupwindow/popupwebpage.cpp \
    popupwindow/popupwebview.cpp \
    popupwindow/popupwindow.cpp \
    preferences/acceptlanguage.cpp \
    preferences/autofillmanager.cpp \
    preferences/jsoptions.cpp \
    preferences/pluginlistdelegate.cpp \
    preferences/pluginsmanager.cpp \
    preferences/preferences.cpp \
    preferences/sslmanager.cpp \
    preferences/thememanager.cpp \
    preferences/useragentdialog.cpp \
    rss/rssicon.cpp \
    rss/rssmanager.cpp \
    rss/rssnotification.cpp \
    rss/rsswidget.cpp \
    session/recoverywidget.cpp \
    session/restoremanager.cpp \
    sidebar/bookmarkssidebar.cpp \
    sidebar/historysidebar.cpp \
    sidebar/sidebar.cpp \
    tabwidget/combotabbar.cpp \
    tabwidget/tabbar.cpp \
    tabwidget/tabicon.cpp \
    tabwidget/tabpreview.cpp \
    tabwidget/tabstackedwidget.cpp \
    tabwidget/tabwidget.cpp \
    tools/aesinterface.cpp \
    tools/animatedwidget.cpp \
    tools/buttonbox.cpp \
    tools/buttonwithmenu.cpp \
    tools/certificateinfowidget.cpp \
    tools/clickablelabel.cpp \
    tools/closedtabsmanager.cpp \
    tools/colors.cpp \
    tools/delayedfilewatcher.cpp \
    tools/docktitlebarwidget.cpp \
    tools/emptynetworkreply.cpp \
    tools/enhancedmenu.cpp \
    tools/focusselectlineedit.cpp \
    tools/followredirectreply.cpp \
    tools/frame.cpp \
    tools/headerview.cpp \
    tools/horizontallistwidget.cpp \
    tools/htmlhighlighter.cpp \
    tools/html5permissions/html5permissionsdialog.cpp \
    tools/html5permissions/html5permissionsmanager.cpp \
    tools/html5permissions/html5permissionsnotification.cpp \
    tools/iconfetcher.cpp \
    tools/iconprovider.cpp \
    tools/json.cpp \
    tools/listitemdelegate.cpp \
    tools/mactoolbutton.cpp \
    tools/menubar.cpp \
    tools/pagethumbnailer.cpp \
    tools/plaineditwithlines.cpp \
    tools/progressbar.cpp \
    tools/qztools.cpp \
    tools/sqldatabase.cpp \
    tools/toolbutton.cpp \
    tools/treewidget.cpp \
    tools/widget.cpp \
    webkit/javascript/externaljsobject.cpp \
    webkit/loadrequest.cpp \
    webkit/webinspector.cpp \
    webkit/webpage.cpp \
    webkit/webpluginfactory.cpp \
    webkit/webview.cpp \
    webtab/searchtoolbar.cpp \
    webtab/tabbedwebview.cpp \
    webtab/webtab.cpp \

HEADERS  += \
    3rdparty/ecwin7.h \
    3rdparty/fancytabwidget.h \
    3rdparty/lineedit.h \
    3rdparty/msvc2008.h \
    3rdparty/processinfo.h \
    3rdparty/qtwin.h \
    3rdparty/squeezelabelv1.h \
    3rdparty/squeezelabelv2.h \
    3rdparty/stylehelper.h \
    adblock/adblockaddsubscriptiondialog.h \
    adblock/adblockblockednetworkreply.h \
    adblock/adblockdialog.h \
    adblock/adblockicon.h \
    adblock/adblockmanager.h \
    adblock/adblockrule.h \
    adblock/adblocksearchtree.h \
    adblock/adblocksubscription.h \
    adblock/adblocktreewidget.h \
    app/autosaver.h \
    app/browserwindow.h \
    app/commandlineoptions.h \
    app/datapaths.h \
    app/mainapplication.h \
    app/mainmenu.h \
    app/profilemanager.h \
    app/proxystyle.h \
    app/qzcommon.h \
    app/settings.h \
    autofill/autofill.h \
    autofill/autofillicon.h \
    autofill/autofillnotification.h \
    autofill/autofillwidget.h \
    autofill/pageformcompleter.h \
    autofill/passwordbackends/databaseencryptedpasswordbackend.h \
    autofill/passwordbackends/databasepasswordbackend.h \
    autofill/passwordbackends/passwordbackend.h \
    autofill/passwordmanager.h \
    bookmarks/bookmarkitem.h \
    bookmarks/bookmarksexport/bookmarksexportdialog.h \
    bookmarks/bookmarksexport/bookmarksexporter.h \
    bookmarks/bookmarksexport/htmlexporter.h \
    bookmarks/bookmarks.h \
    bookmarks/bookmarksicon.h \
    bookmarks/bookmarksimport/bookmarksimportdialog.h \
    bookmarks/bookmarksimport/bookmarksimporter.h \
    bookmarks/bookmarksimport/firefoximporter.h \
    bookmarks/bookmarksimport/htmlimporter.h \
    bookmarks/bookmarksimport/chromeimporter.h \
    bookmarks/bookmarksimport/ieimporter.h \
    bookmarks/bookmarksimport/operaimporter.h \
    bookmarks/bookmarksitemdelegate.h \
    bookmarks/bookmarksmanager.h \
    bookmarks/bookmarksmenu.h \
    bookmarks/bookmarksmodel.h \
    bookmarks/bookmarkstoolbarbutton.h \
    bookmarks/bookmarkstoolbar.h \
    bookmarks/bookmarkstools.h \
    bookmarks/bookmarkstreeview.h \
    bookmarks/bookmarkswidget.h \
    cookies/cookiejar.h \
    cookies/cookiemanager.h \
    downloads/downloadfilehelper.h \
    downloads/downloaditem.h \
    downloads/downloadmanager.h \
    downloads/downloadoptionsdialog.h \
    history/history.h \
    history/historyitem.h \
    history/historymanager.h \
    history/historymenu.h \
    history/historymodel.h \
    history/historytreeview.h \
    history/webhistoryinterface.h \
    navigation/completer/locationcompleterdelegate.h \
    navigation/completer/locationcompleter.h \
    navigation/completer/locationcompletermodel.h \
    navigation/completer/locationcompleterrefreshjob.h \
    navigation/completer/locationcompleterview.h \
    navigation/downicon.h \
    navigation/goicon.h \
    navigation/locationbar.h \
    navigation/locationbarpopup.h \
    navigation/navigationbar.h \
    navigation/navigationcontainer.h \
    navigation/reloadstopbutton.h \
    navigation/siteicon.h \
    navigation/websearchbar.h \
    network/cabundleupdater.h \
    network/networkmanager.h \
    network/networkmanagerproxy.h \
    network/networkproxyfactory.h \
    network/pac/pacdatetime.h \
    network/pac/pacmanager.h \
    network/pac/proxyautoconfig.h \
    network/schemehandlers/adblockschemehandler.h \
    network/schemehandlers/fileschemehandler.h \
    network/schemehandlers/ftpschemehandler.h \
    network/schemehandlers/qupzillaschemehandler.h \
    network/schemehandlers/schemehandler.h \
    network/sslerrordialog.h \
    notifications/desktopnotification.h \
    notifications/desktopnotificationsfactory.h \
    opensearch/editsearchengine.h \
    opensearch/opensearchenginedelegate.h \
    opensearch/opensearchengine.h \
    opensearch/opensearchreader.h \
    opensearch/searchenginesdialog.h \
    opensearch/searchenginesmanager.h \
    other/aboutdialog.h \
    other/browsinglibrary.h \
    other/clearprivatedata.h \
    other/checkboxdialog.h \
    other/iconchooser.h \
    other/licenseviewer.h \
    other/pagescreen.h \
    other/qzsettings.h \
    other/siteinfo.h \
    other/siteinfowidget.h \
    other/sourceviewer.h \
    other/sourceviewersearch.h \
    other/statusbarmessage.h \
    other/updater.h \
    other/useragentmanager.h \
    plugins/clicktoflash.h \
    plugins/plugininterface.h \
    plugins/pluginproxy.h \
    plugins/plugins.h \
    plugins/speeddial.h \
    popupwindow/popuplocationbar.h \
    popupwindow/popupstatusbarmessage.h \
    popupwindow/popupwebpage.h \
    popupwindow/popupwebview.h \
    popupwindow/popupwindow.h \
    preferences/acceptlanguage.h \
    preferences/autofillmanager.h \
    preferences/jsoptions.h \
    preferences/pluginlistdelegate.h \
    preferences/pluginsmanager.h \
    preferences/preferences.h \
    preferences/sslmanager.h \
    preferences/thememanager.h \
    preferences/useragentdialog.h \
    rss/rssicon.h \
    rss/rssmanager.h \
    rss/rssnotification.h \
    rss/rsswidget.h \
    session/recoverywidget.h \
    session/restoremanager.h \
    sidebar/bookmarkssidebar.h \
    sidebar/historysidebar.h \
    sidebar/sidebar.h \
    sidebar/sidebarinterface.h \
    tabwidget/combotabbar.h \
    tabwidget/tabbar.h \
    tabwidget/tabicon.h \
    tabwidget/tabpreview.h \
    tabwidget/tabstackedwidget.h \
    tabwidget/tabwidget.h \
    tools/aesinterface.h \
    tools/animatedwidget.h \
    tools/buttonbox.h \
    tools/buttonwithmenu.h \
    tools/certificateinfowidget.h \
    tools/clickablelabel.h \
    tools/closedtabsmanager.h \
    tools/colors.h \
    tools/delayedfilewatcher.h \
    tools/docktitlebarwidget.h \
    tools/emptynetworkreply.h \
    tools/enhancedmenu.h \
    tools/focusselectlineedit.h \
    tools/followredirectreply.h \
    tools/frame.h \
    tools/headerview.h \
    tools/horizontallistwidget.h \
    tools/htmlhighlighter.h \
    tools/html5permissions/html5permissionsdialog.h \
    tools/html5permissions/html5permissionsmanager.h \
    tools/html5permissions/html5permissionsnotification.h \
    tools/iconfetcher.h \
    tools/iconprovider.h \
    tools/json.h \
    tools/listitemdelegate.h \
    tools/mactoolbutton.h \
    tools/menubar.h \
    tools/pagethumbnailer.h \
    tools/plaineditwithlines.h \
    tools/progressbar.h \
    tools/qztools.h \
    tools/sqldatabase.h \
    tools/toolbutton.h \
    tools/treewidget.h \
    tools/widget.h \
    webkit/javascript/externaljsobject.h \
    webkit/loadrequest.h \
    webkit/webinspector.h \
    webkit/webpage.h \
    webkit/webpluginfactory.h \
    webkit/webview.h \
    webtab/searchtoolbar.h \
    webtab/tabbedwebview.h \
    webtab/webtab.h \

FORMS    += \
    adblock/adblockaddsubscriptiondialog.ui \
    adblock/adblockdialog.ui \
    autofill/autofillnotification.ui \
    autofill/autofillwidget.ui \
    autofill/passwordbackends/masterpassworddialog.ui \
    bookmarks/bookmarksexport/bookmarksexportdialog.ui \
    bookmarks/bookmarksimport/bookmarksimportdialog.ui \
    bookmarks/bookmarksmanager.ui \
    bookmarks/bookmarkswidget.ui \
    cookies/cookiemanager.ui \
    downloads/downloaditem.ui \
    downloads/downloadmanager.ui \
    downloads/downloadoptionsdialog.ui \
    history/historymanager.ui \
    network/sslerrordialog.ui \
    notifications/desktopnotification.ui \
    opensearch/editsearchengine.ui \
    opensearch/searchenginesdialog.ui \
    other/aboutdialog.ui \
    other/browsinglibrary.ui \
    other/clearprivatedata.ui \
    other/checkboxdialog.ui \
    other/iconchooser.ui \
    other/pagescreen.ui \
    other/siteinfo.ui \
    other/siteinfowidget.ui \
    other/sourceviewersearch.ui \
    preferences/acceptlanguage.ui \
    preferences/addacceptlanguage.ui \
    preferences/autofillmanager.ui \
    preferences/jsoptions.ui \
    preferences/pluginslist.ui \
    preferences/preferences.ui \
    preferences/sslmanager.ui \
    preferences/thememanager.ui \
    preferences/useragentdialog.ui \
    rss/rssmanager.ui \
    rss/rssnotification.ui \
    rss/rsswidget.ui \
    session/recoverywidget.ui \
    sidebar/bookmarkssidebar.ui \
    sidebar/historysidebar.ui \
    tools/certificateinfowidget.ui \
    tools/docktitlebarwidget.ui \
    tools/html5permissions/html5permissionsdialog.ui \
    tools/html5permissions/html5permissionsnotification.ui \
    webkit/jsalert.ui \
    webkit/jsconfirm.ui \
    webkit/jsprompt.ui \
    webtab/searchtoolbar.ui \

RESOURCES += \
    data/certs.qrc \
    data/data.qrc \
    data/html.qrc \
    data/icons.qrc \
    data/oxygen-fallback.qrc

isEqual(QT_MAJOR_VERSION, 5) {
    include(3rdparty/qftp/qftp.pri)
    SOURCES += tools/qzregexp.cpp
}

!mac:unix {
    target.path = $$library_folder

    INSTALLS += target

    !contains(DEFINES, NO_X11):LIBS += -lX11
    LIBS += -lcrypto

    RESOURCES -= data/certs.qrc
}

win32 {
    HEADERS += other/registerqappassociation.h
    SOURCES += other/registerqappassociation.cpp

    LIBS += -llibeay32
}

os2 {
    LIBS += -lcrypto
}

openbsd-*|freebsd-*|haiku-* {
    LIBS += -lexecinfo
}

mac {
    HEADERS += other/macmenureceiver.h \
               webview/macwebviewscroller.h
    SOURCES += other/macmenureceiver.cpp \
               webview/macwebviewscroller.cpp
    RESOURCES -= data/certs.qrc

    LIBS += -lcrypto -framework CoreServices
}

message(===========================================)
message( Using following defines:)
message(  $$DEFINES)
