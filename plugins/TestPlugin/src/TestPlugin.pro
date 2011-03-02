#-------------------------------------------------
#
# Project created by QtCreator 2011-02-13T10:23:13
#
#-------------------------------------------------
QT += network webkit

TEMPLATE      = lib

CONFIG       += plugin

TARGET        = ExamplePlugin

DESTDIR = ../../../bin/plugins

SOURCES += testplugin.cpp

HEADERS += testplugin.h

RESOURCES += \
    data.qrc
