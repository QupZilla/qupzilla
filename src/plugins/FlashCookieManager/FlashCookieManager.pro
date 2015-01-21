TARGET = $$qtLibraryTarget(FlashCookieManager)
os2: TARGET = FlashCoo

SOURCES += fcm_plugin.cpp \
    fcm_dialog.cpp \
    fcm_notification.cpp

HEADERS += fcm_plugin.h \
    fcm_dialog.h \
    fcm_notification.h

RESOURCES += flashcookiemanager.qrc

TRANSLATIONS += \
    translations/fa_IR.ts

PLUGIN_DIR = $$PWD
srcdir = $$(QUPZILLA_SRCDIR)
equals(srcdir, "") {
    include(../../plugins.pri)
}
else {
    include($$srcdir/src/plugins.pri)
}

FORMS += \
    fcm_dialog.ui \
    fcm_notification.ui
