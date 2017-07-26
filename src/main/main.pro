include(../defines.pri)

QT += webenginecore webenginewidgets network widgets sql

TARGET = qupzilla
mac: TARGET = QupZilla

TEMPLATE = app

compile_libtool {
LIBS += $$QZ_DESTDIR/libQupZilla.la
}
else {
!unix|mac: LIBS += -L$$QZ_DESTDIR -lQupZilla
!mac:unix: LIBS += $$QZ_DESTDIR/libQupZilla.so
}

unix:!contains(DEFINES, "DISABLE_DBUS") QT += dbus

INCLUDEPATH += ../lib/3rdparty \
               ../lib/app \
               ../lib/session \
               ../lib/webengine \
               ../lib/webtab \

DEPENDPATH += $$INCLUDEPATH

SOURCES = main.cpp

OTHER_FILES += appicon.rc \
               appicon_os2.rc \
               Info.plist \

os2:RC_FILE = appicon_os2.rc
win32:RC_FILE = appicon.rc

openbsd-*|freebsd-*|haiku-* {
    LIBS += -lexecinfo
}

include(../install.pri)

unix:contains(DEFINES, "NO_SYSTEM_DATAPATH") {
   # For running the app without installing it
   QMAKE_LFLAGS+=$${QMAKE_LFLAGS_RPATH}\\$\$ORIGIN
   # For running the app after installation
   QMAKE_LFLAGS+=$${QMAKE_LFLAGS_RPATH}$${library_folder}
   message(QMAKE_LFLAGS: $$QMAKE_LFLAGS)
}
