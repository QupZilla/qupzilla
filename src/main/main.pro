QT += core gui webkit sql network script

TARGET = qupzilla
mac: TARGET = QupZilla

TEMPLATE = app

!unix|mac: LIBS += -L../../bin -lQupZilla
!mac:unix: LIBS += ../../bin/libQupZilla.so

include(../defines.pri)
include(../install.pri)

unix:!contains(DEFINES, "DISABLE_DBUS") QT += dbus

INCLUDEPATH += ../lib/app\
               ../lib/3rdparty

SOURCES =      main.cpp
OTHER_FILES += appicon.rc \
               appicon_os2.rc \
               Info.plist

os2:RC_FILE = appicon_os2.rc
win32:RC_FILE = appicon.rc

unix:contains(DEFINES, "NO_SYSTEM_DATAPATH"): QMAKE_LFLAGS+=$${QMAKE_LFLAGS_RPATH}\\$\$ORIGIN
