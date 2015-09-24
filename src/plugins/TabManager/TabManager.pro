TARGET = $$qtLibraryTarget(TabManager)
# OS/2 allows only 8 chars in TARGET
os2: TARGET = TabManPl

SOURCES += tabmanagerplugin.cpp \
    tabmanagerwidget.cpp \
    tabmanagerwidgetcontroller.cpp

HEADERS += tabmanagerplugin.h \
    tabmanagerwidget.h \
    tabmanagerwidgetcontroller.h

RESOURCES += tabmanagerplugin.qrc

FORMS += \
    tabmanagerwidget.ui

TRANSLATIONS += \
    translations/cs_CZ.ts \
    translations/da_DK.ts \
    translations/de_DE.ts \
    translations/el_GR.ts \
    translations/es_ES.ts \
    translations/es_MX.ts \
    translations/eu_ES.ts \
    translations/fa_IR.ts \
    translations/fr_FR.ts \
    translations/he_IL.ts \
    translations/hr_HR.ts \
    translations/id_ID.ts \
    translations/it_IT.ts \
    translations/lt.ts \
    translations/nl_NL.ts \
    translations/pl_PL.ts \
    translations/pt_PT.ts \
    translations/ru_RU.ts \
    translations/sr@ijekavianlatin.ts \
    translations/sr@ijekavian.ts \
    translations/sr@latin.ts \
    translations/sr.ts \
    translations/tr_TR.ts \
    translations/zh_CN.ts \
    translations/zh_TW.ts \

include(tldextractor/tldextractor.pri)

PLUGIN_DIR = $$PWD
include(../../plugins.pri)
