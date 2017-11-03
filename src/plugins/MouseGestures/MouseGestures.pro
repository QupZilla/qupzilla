TARGET = $$qtLibraryTarget(MouseGestures)
os2: TARGET = MouseGes

INCLUDEPATH = 3rdparty
DEPENDPATH = 3rdparty

SOURCES = \
    3rdparty/mousegesturerecognizer.cpp \
    3rdparty/QjtMouseGesture.cpp \
    3rdparty/QjtMouseGestureFilter.cpp \
    3rdparty/adv_recognizer.cpp \
    mousegestures.cpp \
    mousegesturesplugin.cpp \
    mousegesturessettingsdialog.cpp

HEADERS = \
    3rdparty/QjtMouseGesture.h \
    3rdparty/QjtMouseGestureFilter.h \
    mousegestures.h \
    mousegesturesplugin.h \
    mousegesturessettingsdialog.h

FORMS += \
    mousegesturessettingsdialog.ui

RESOURCES = mousegestures.qrc

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
    translations/es_VE.ts \
    translations/eu_ES.ts \
    translations/fa_IR.ts \
    translations/fi_FI.ts \
    translations/fr_FR.ts \
    translations/gl_ES.ts \
    translations/he_IL.ts \
    translations/hr_HR.ts \
    translations/id_ID.ts \
    translations/is.ts \
    translations/it_IT.ts \
    translations/ja_JP.ts \
    translations/ka_GE.ts \
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
    translations/sv_SE.ts \
    translations/te.ts \
    translations/tr_TR.ts \
    translations/uk_UA.ts \
    translations/uz@Cyrl.ts \
    translations/uz@Latn.ts \
    translations/zh_CN.ts \
    translations/zh_HK.ts \
    translations/zh_TW.ts \

PLUGIN_DIR = $$PWD
include(../../plugins.pri)
