QT += core gui webkit sql network script
unix:QT += dbus
TARGET = qupzilla
TEMPLATE = app

include(3rdparty/qtsingleapplication.pri)
include(src.pri)
include(../install.pri)
include(../defines.pri)
include(../translations.pri)

message(Using following defines)
message($$DEFINES)
