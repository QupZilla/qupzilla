include(../../src/3rdparty/qtsingleapplication.pri)
include(../../defines.pri)
include(../../install.pri)
include(../../src/src.pri)

QT += core gui webkit sql network script
unix:QT += dbus

TARGET = qupzilla
TEMPLATE = lib
CONFIG -= shared
CONFIG += static

DESTDIR = $$PWD
OBJECTS_DIR = $$PWD/build
RESOURCES =
win32|os2:RC_FILE =
