TARGET = $$qtLibraryTarget(AutoScroll)
# OS/2 allows only 8 chars in TARGET
os2: TARGET = AutoScrl

SOURCES += autoscrollplugin.cpp \
    autoscroller.cpp \
    framescroller.cpp \
    autoscrollsettings.cpp

HEADERS += autoscrollplugin.h \
    autoscroller.h \
    framescroller.h \
    autoscrollsettings.h

FORMS += \
    autoscrollsettings.ui

RESOURCES += autoscroll.qrc

TRANSLATIONS += \
    translations/ar_SA.ts \
    translations/bg_BG.ts \
    translations/ca_ES.ts \
    translations/cs_CZ.ts \
    translations/da_DK.ts \
    translations/de_DE.ts \
    translations/el_GR.ts \
    translations/es_ES.ts \
    translations/es_MX.ts \
    translations/eu_ES.ts \
    translations/fi_FI.ts \
    translations/fr_FR.ts \
    translations/he_IL.ts \
    translations/hr_HR.ts \
    translations/hu_HU.ts \
    translations/id_ID.ts \
    translations/is.ts \
    translations/it_IT.ts \
    translations/ja_JP.ts \
    translations/lt.ts \
    translations/lv_LV.ts \
    translations/nb_NO.ts \
    translations/nl_NL.ts \
    translations/nqo.ts \
    translations/pl_PL.ts \
    translations/pt_BR.ts \
    translations/pt_PT.ts \
    translations/ro_RO.ts \
    translations/ru_RU.ts \
    translations/sk_SK.ts \
    translations/sl_SI.ts \
    translations/sr@ijekavianlatin.ts \
    translations/sr@ijekavian.ts \
    translations/sr@latin.ts \
    translations/sr.ts \
    translations/tr_TR.ts \
    translations/uk_UA.ts \
    translations/zh_CN.ts \
    translations/zh_HK.ts \
    translations/zh_TW.ts \

PLUGIN_DIR = $$PWD
include(../../plugins.pri)
