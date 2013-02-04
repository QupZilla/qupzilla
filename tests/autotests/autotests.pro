isEqual(QT_MAJOR_VERSION, 5) {
    QT += webkitwidgets network widgets printsupport sql script gui-private
} else {
    QT += core gui webkit sql network script
}

TARGET = autotests
CONFIG += qtestlib

!unix|mac: LIBS += -L$$PWD/../../bin -lQupZilla
!mac:unix: LIBS += $$PWD/../../bin/libQupZilla.so

unix:contains(DEFINES, "NO_SYSTEM_DATAPATH"): QMAKE_LFLAGS+=$${QMAKE_LFLAGS_RPATH}\\$\$ORIGIN

include($$PWD/../../src/defines.pri)

DESTDIR = $$PWD/../../bin
OBJECTS_DIR = build
MOC_DIR = build
RCC_DIR = build
UI_DIR = build

INCLUDEPATH += $$PWD/../../src/lib/3rdparty\
               $$PWD/../../src/lib/app\
               $$PWD/../../src/lib/autofill\
               $$PWD/../../src/lib/bookmarks\
               $$PWD/../../src/lib/cookies\
               $$PWD/../../src/lib/session\
               $$PWD/../../src/lib/downloads\
               $$PWD/../../src/lib/history\
               $$PWD/../../src/lib/navigation\
               $$PWD/../../src/lib/network\
               $$PWD/../../src/lib/other\
               $$PWD/../../src/lib/preferences\
               $$PWD/../../src/lib/rss\
               $$PWD/../../src/lib/tools\
               $$PWD/../../src/lib/utils\
               $$PWD/../../src/lib/webview\
               $$PWD/../../src/lib/plugins\
               $$PWD/../../src/lib/sidebar\
               $$PWD/../../src/lib/data\
               $$PWD/../../src/lib/adblock\
               $$PWD/../../src/lib/desktopnotifications\
               $$PWD/../../src/lib/opensearch\
               $$PWD/../../src/lib/bookmarksimport\
               $$PWD/../../src/lib/popupwindow\

HEADERS += \
    qztoolstest.h \
    formcompletertest.h \
    cookiestest.h \
    downloadstest.h

SOURCES += \
    qztoolstest.cpp \
    main.cpp \
    formcompletertest.cpp \
    cookiestest.cpp \
    downloadstest.cpp
