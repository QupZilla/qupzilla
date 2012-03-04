QT += webkit
TARGET = AccessKeysNavigation

SOURCES = \
    akn_plugin.cpp \
    akn_handler.cpp \
    akn_settings.cpp

HEADERS = \
    akn_plugin.h \
    akn_handler.h \
    akn_settings.h

FORMS += \
    akn_settings.ui

RESOURCES = akn_res.qrc

TRANSLATIONS = translations/cs_CZ.ts \

include(../../plugins.pri)
