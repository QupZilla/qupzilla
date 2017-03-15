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
    translations/de_DE.ts \
    translations/el_GR.ts \
    translations/es_ES.ts \
    translations/es_MX.ts \
    translations/eu_ES.ts \
    translations/fr_FR.ts \
    translations/hr_HR.ts \
    translations/is.ts \
    translations/it_IT.ts \
    translations/lt.ts \
    translations/nl_NL.ts \
    translations/pt_PT.ts \
    translations/ru_RU.ts \
    translations/uk_UA.ts \
    translations/zh_TW.ts \

PLUGIN_DIR = $$PWD
include(../../plugins.pri)
