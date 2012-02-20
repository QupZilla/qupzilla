#-------------------------------------------------
#
# Project created by QtCreator 2011-02-13T10:23:13
#
#-------------------------------------------------
QT += network webkit sql
TARGET = ExamplePlugin

SOURCES += testplugin.cpp
HEADERS += testplugin.h
RESOURCES += \
    testplugin.qrc

TRANSLATIONS += cs_CZ.ts\
                sk_SK.ts\
                sr_BA.ts\
                sr_RS.ts\
                de_DE.ts\
                el_GR.ts\

include(../../plugins.pri)
