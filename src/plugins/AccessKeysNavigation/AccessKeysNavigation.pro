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

TRANSLATIONS += \
    translations/ar_SA.ts \
    translations/bg_BG.ts \
    translations/bo_CN.ts \
    translations/ca_ES.ts \
    translations/cs_CZ.ts \
    translations/da_DK.ts \
    translations/de_DE.ts \
    translations/el_GR.ts \
    translations/es_419.ts \
    translations/es_AR.ts \
    translations/es_ES.ts \
    translations/es_MX.ts \
    translations/es_VE.ts \
    translations/eu_ES.ts \
    translations/fa_IR.ts \
    translations/fi_FI.ts \
    translations/fr_FR.ts \
    translations/gl_ES.ts \
    translations/he_IL.ts \
    translations/hi_IN.ts \
    translations/hi.ts \
    translations/hr_HR.ts \
    translations/hu_HU.ts \
    translations/id_ID.ts \
    translations/it_IT.ts \
    translations/ja_JP.ts \
    translations/ka_GE.ts \
    translations/lt.ts \
    translations/lv_LV.ts \
    translations/my_MM.ts \
    translations/nb_NO.ts \
    translations/nl_NL.ts \
    translations/nqo.ts \
    translations/pl_PL.ts \
    translations/pt_BR.ts \
    translations/pt_PT.ts \
    translations/ro_RO.ts \
    translations/ru_RU.ts \
    translations/sk_SK.ts \
    translations/sr@ijekavianlatin.ts \
    translations/sr@ijekavian.ts \
    translations/sr@latin.ts \
    translations/sr.ts \
    translations/sv_SE.ts \
    translations/tr_TR.ts \
    translations/uk_UA.ts \
    translations/uz@Cyrl.ts \
    translations/uz@Latn.ts \
    translations/zh_CN.ts \
    translations/zh_HK.ts \
    translations/zh_TW.ts \

PLUGIN_DIR = $$PWD
include(../../plugins.pri)
