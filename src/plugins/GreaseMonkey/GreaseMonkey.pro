TARGET = GreaseMonkey
os2: TARGET = GreaseMo

SOURCES += gm_plugin.cpp \
    gm_manager.cpp \
    gm_script.cpp \
    gm_urlmatcher.cpp \
    gm_downloader.cpp \
    gm_addscriptdialog.cpp \
    gm_notification.cpp \
    settings/gm_settings.cpp \
    settings/gm_settingslistdelegate.cpp \
    settings/gm_settingsscriptinfo.cpp \
    settings/gm_settingslistwidget.cpp

HEADERS += gm_plugin.h \
    gm_manager.h \
    gm_script.h \
    gm_urlmatcher.h \
    gm_downloader.h \
    gm_addscriptdialog.h \
    gm_notification.h \
    settings/gm_settings.h \
    settings/gm_settingslistdelegate.h \
    settings/gm_settingsscriptinfo.h \
    settings/gm_settingslistwidget.h

FORMS += \
    gm_addscriptdialog.ui \
    gm_notification.ui \
    settings/gm_settings.ui \
    settings/gm_settingsscriptinfo.ui

RESOURCES += greasemonkey.qrc

TRANSLATIONS = \
    translations/cs_CZ.ts \
    translations/de_DE.ts \
    translations/fr_FR.ts \
    translations/it_IT.ts \
    translations/sr_BA.ts \
    translations/sr_RS.ts \

srcdir = $$(QUPZILLA_SRCDIR)
equals(srcdir, "") {
    include(../../plugins.pri)
}
else {
    include($$srcdir/src/plugins.pri)
}
