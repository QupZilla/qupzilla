TARGET = $$qtLibraryTarget(MouseGestures)
os2: TARGET = MouseGes

INCLUDEPATH = 3rdparty

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
    translations/sk_SK.ts \
    translations/sr_BA.ts \
    translations/sr_BA@latin.ts \
    translations/sr_RS.ts \
    translations/sr_RS@latin.ts \
    translations/sv_SE.ts \
    translations/uk_UA.ts \
    translations/zh_TW.ts \
    translations/fa_IR.ts \

include(../../plugins.pri)
