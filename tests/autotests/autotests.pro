include($$PWD/../../src/defines.pri)

QT += webenginewidgets network widgets printsupport sql script testlib

TARGET = autotests

!unix|mac: LIBS += -L$$PWD/../../bin -lQupZilla
!mac:unix: LIBS += $$PWD/../../bin/libQupZilla.so

QMAKE_LFLAGS+=$${QMAKE_LFLAGS_RPATH}$$PWD/../../bin

# KWallet plugin
exists($$PWD/../../bin/plugins/libKWalletPasswords.so) {
    isEqual(QT_MAJOR_VERSION, 4) | qtHaveModule(KWallet) {
        LIBS += $$PWD/../../bin/plugins/libKWalletPasswords.so
        DEFINES += HAVE_KDE_PASSWORDS_PLUGIN
    }
}

# GnomeKeyring plugin
exists($$PWD/../../bin/plugins/libGnomeKeyringPasswords.so) {
    LIBS += $$PWD/../../bin/plugins/libGnomeKeyringPasswords.so
    DEFINES += HAVE_GNOME_PASSWORDS_PLUGIN
}

DESTDIR =
OBJECTS_DIR = build
MOC_DIR = build
RCC_DIR = build
UI_DIR = build

INCLUDEPATH += $$PWD/../../src/lib/3rdparty \
               $$PWD/../../src/lib/adblock \
               $$PWD/../../src/lib/app \
               $$PWD/../../src/lib/autofill \
               $$PWD/../../src/lib/bookmarks \
               $$PWD/../../src/lib/cookies \
               $$PWD/../../src/lib/downloads \
               $$PWD/../../src/lib/history \
               $$PWD/../../src/lib/navigation \
               $$PWD/../../src/lib/network \
               $$PWD/../../src/lib/notifications \
               $$PWD/../../src/lib/opensearch \
               $$PWD/../../src/lib/other \
               $$PWD/../../src/lib/plugins \
               $$PWD/../../src/lib/popupwindow \
               $$PWD/../../src/lib/preferences \
               $$PWD/../../src/lib/session \
               $$PWD/../../src/lib/sidebar \
               $$PWD/../../src/lib/tabwidget \
               $$PWD/../../src/lib/tools \
               $$PWD/../../src/lib/webengine \
               $$PWD/../../src/lib/webtab \

HEADERS += \
    qztoolstest.h \
    cookiestest.h \
    adblocktest.h \
    updatertest.h \
    pactest.h \
    passwordbackendtest.h \
    networktest.h

SOURCES += \
    qztoolstest.cpp \
    main.cpp \
    cookiestest.cpp \
    adblocktest.cpp \
    updatertest.cpp \
    pactest.cpp \
    passwordbackendtest.cpp \
    networktest.cpp
