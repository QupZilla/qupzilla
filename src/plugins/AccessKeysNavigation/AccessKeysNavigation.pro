QT += webkit
TARGET = AccessKeysNavigation
os2: TARGET  = AcKeyNav

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

TRANSLATIONS = \
    translations/cs_CZ.ts \
    translations/de_DE.ts \
    translations/el_GR.ts \
    translations/es_ES.ts \
    translations/id_ID.ts \
    translations/ja_JP.ts \
    translations/nl_NL.ts \
    translations/pt_PT.ts \
    translations/ro_RO.ts \
    translations/sr_BA.ts \
    translations/sr_RS.ts \
    translations/sv_SE.ts \
    translations/zh_TW.ts \

include(../../plugins.pri)
