QT += core gui webkit sql network script
unix: QT += dbus

TARGET = qupzilla
TEMPLATE = app
!unix|mac: LIBS += -L../../bin -lqupzilla
!mac:unix: LIBS += ../../bin/libqupzilla.so

include(../defines.pri)
include(../install.pri)

INCLUDEPATH += ../lib/app\
               ../lib/3rdparty

SOURCES =      main.cpp
OTHER_FILES += appicon.rc \
               appicon_os2.rc \
               Info.plist

os2:RC_FILE = appicon_os2.rc
win32:RC_FILE = appicon.rc

unix:contains(DEFINES, "NO_SYSTEM_DATAPATH"): QMAKE_RPATHDIR += $$PWD/../../bin

message(========== Building qupzilla binary ==========)
