QT += webkit
TARGET = MouseGestures
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
    translations/id_ID.ts \
    translations/ja_JP.ts \
    translations/nl_NL.ts \
    translations/sk_SK.ts \
    translations/sr_BA.ts \
    translations/sr_RS.ts \
    translations/sv_SE.ts \
    translations/zh_TW.ts \

include(../../plugins.pri)
