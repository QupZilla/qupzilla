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

FORMS += \
    fcm_dialog.ui \
    fcm_notification.ui

PLUGIN_DIR = $$PWD
include(../../plugins.pri)
