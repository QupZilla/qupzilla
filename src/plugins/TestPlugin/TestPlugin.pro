#-------------------------------------------------
#
# Project created by QtCreator 2011-02-13T10:23:13
#
#-------------------------------------------------
QT += network webkit sql
TARGET = ExamplePlugin

SOURCES += testplugin.cpp
HEADERS += testplugin.h
RESOURCES += testplugin.qrc

TRANSLATIONS += translations/cs_CZ.ts\
                translations/sk_SK.ts\
                translations/sr_BA.ts\
                translations/sr_RS.ts\
                translations/de_DE.ts\
                translations/el_GR.ts\
                translations/id_ID.ts\
                translations/zh_CN.ts\
                translations/zh_TW.ts\

include(../../plugins.pri)
