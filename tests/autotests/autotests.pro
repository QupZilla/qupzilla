include($$PWD/../../src/defines.pri)

QT += webenginewidgets network widgets printsupport sql script dbus testlib

TARGET = autotests

CONFIG -= app_bundle

!unix|mac: LIBS += -L$$PWD/../../bin -lQupZilla
!mac:unix: LIBS += $$PWD/../../bin/libQupZilla.so

QMAKE_LFLAGS+=$${QMAKE_LFLAGS_RPATH}$$PWD/../../bin

# KWallet plugin
exists($$PWD/../../bin/plugins/libKWalletPasswords.so) {
    LIBS += $$PWD/../../bin/plugins/libKWalletPasswords.so
    DEFINES += HAVE_KDE_PASSWORDS_PLUGIN
}

# GnomeKeyring plugin
exists($$PWD/../../bin/plugins/libGnomeKeyringPasswords.so) {
    LIBS += $$PWD/../../bin/plugins/libGnomeKeyringPasswords.so
    DEFINES += HAVE_GNOME_PASSWORDS_PLUGIN
}

mac {
    # homebrew openssl
    BREW_OPENSSL = $$system("brew --prefix openssl")
    INCLUDEPATH += $$BREW_OPENSSL/include
    LIBS += -L$$BREW_OPENSSL/lib

    LIBS += -lcrypto -framework CoreServices
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
    passwordbackendtest.h \

SOURCES += \
    qztoolstest.cpp \
    main.cpp \
    cookiestest.cpp \
    adblocktest.cpp \
    updatertest.cpp \
    passwordbackendtest.cpp \
