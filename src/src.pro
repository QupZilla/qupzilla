QT += core gui webkit sql network script
unix:QT += dbus
TARGET = qupzilla
TEMPLATE = lib

include(3rdparty/qtsingleapplication.pri)
include(src.pri)
include(../defines.pri)

SOURCES -= main.cpp

message(========== Building libqupzilla ==========)
message( Using following defines:)
message($$DEFINES)
