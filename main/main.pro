QT += core gui webkit sql network script
unix: QT += dbus

TARGET = qupzilla
TEMPLATE = app
LIBS += -L../bin -lqupzilla

include(../install.pri)
include(../defines.pri)
include(../translations.pri)
include(../src/3rdparty/qtsingleapplication.pri)

INCLUDEPATH += ../src/app
SOURCES = ../src/main.cpp

unix:contains(DEFINES, "NO_SYSTEM_DATAPATH"): QMAKE_RPATHDIR += $$PWD/../bin

message(========== Building qupzilla binary ==========)
