#-------------------------------------------------
#
# Project created by QtCreator 2010-12-18T14:53:41
#
#-------------------------------------------------

QT += core gui webkit sql network script
unix:QT += dbus
TARGET = qupzilla
TEMPLATE = app


#static_library {
#    TEMPLATE = lib
#    CONFIG -= shared
#    CONFIG += static
#}

include(3rdparty/qtsingleapplication.pri)
include(src.pri)
include(../install.pri)
include(../defines.pri)
include(../translations.pri)

message(Using following defines)
message($$DEFINES)
