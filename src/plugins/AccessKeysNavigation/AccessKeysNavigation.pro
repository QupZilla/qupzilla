TARGET = $$qtLibraryTarget(AccessKeysNavigation)
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
    translations/es_VE.ts \
    translations/fr_FR.ts \
    translations/id_ID.ts \
    translations/it_IT.ts \
    translations/ja_JP.ts \
    translations/ka_GE.ts \
    translations/nl_NL.ts \
    translations/pt_BR.ts \
    translations/pt_PT.ts \
    translations/ro_RO.ts \
    translations/ru_RU.ts \
    translations/sr_BA.ts \
    translations/sr_BA@latin.ts \
    translations/sr_RS.ts \
    translations/sr_RS@latin.ts \
    translations/sv_SE.ts \
    translations/uk_UA.ts \
    translations/zh_TW.ts \
    translations/fa_IR.ts \

include(../../plugins.pri)
