TARGET = $$qtLibraryTarget(ImageFinder)
# OS/2 allows only 8 chars in TARGET
os2: TARGET = ImgFndr

HEADERS += \
    imagefinderplugin.h \
    imagefinder.h \
    imagefindersettings.h \

SOURCES += \
    imagefinderplugin.cpp \
    imagefinder.cpp \
    imagefindersettings.cpp \

RESOURCES += imagefinder.qrc

FORMS += imagefindersettings.ui

TRANSLATIONS += \
    translations/bg_BG.ts \
    translations/ca_ES.ts \
    translations/cs_CZ.ts \
    translations/da_DK.ts \
    translations/de_DE.ts \
    translations/el_GR.ts \
    translations/es_ES.ts \
    translations/es_MX.ts \
    translations/eu_ES.ts \
    translations/fr_FR.ts \
    translations/hr_HR.ts \
    translations/hu_HU.ts \
    translations/id_ID.ts \
    translations/is.ts \
    translations/it_IT.ts \
    translations/ja_JP.ts \
    translations/lt.ts \
    translations/nb_NO.ts \
    translations/nl_NL.ts \
    translations/pt_PT.ts \
    translations/ru_RU.ts \
    translations/sl_SI.ts \
    translations/tr_TR.ts \
    translations/uk_UA.ts \
    translations/zh_CN.ts \
    translations/zh_TW.ts \

PLUGIN_DIR = $$PWD
include(../../plugins.pri)
