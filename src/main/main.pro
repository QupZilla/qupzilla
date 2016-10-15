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

unix:contains(DEFINES, "NO_SYSTEM_DATAPATH"): QMAKE_LFLAGS+=$${QMAKE_LFLAGS_RPATH}\\$\$ORIGIN

DISTFILES += \
    manifest.xml

win32 {
    WINSDK_DIR = C:/Program Files (x86)/Microsoft SDKs/Windows/v7.1A
    WIN_PWD = $$replace(PWD, /, \\)
    OUT_PWD_WIN = $$replace(DESTDIR, /, \\)
    QMAKE_POST_LINK += \
        "$$WINSDK_DIR/bin/x64/mt.exe -manifest $$quote($$WIN_PWD\\manifest.xml) -outputresource:$$quote($$OUT_PWD_WIN\\$$basename(TARGET).exe;1)"
}
