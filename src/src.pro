QT += core gui webkit sql network script
unix:QT += dbus
TARGET = qupzilla
TEMPLATE = lib

include(3rdparty/qtsingleapplication.pri)
include(src.pri)
include(../defines.pri)

SOURCES -= main.cpp

!mac:unix {
    target.path = $$library_folder

    INSTALLS += target
}

message(========== Building libqupzilla ==========)
message( Using following defines:)
message($$DEFINES)
