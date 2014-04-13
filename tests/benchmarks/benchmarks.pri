include($$PWD/../../src/defines.pri)

isEqual(QT_MAJOR_VERSION, 5) {
    QT += webkitwidgets network widgets printsupport sql script gui-private testlib
} else {
    QT += core gui webkit sql network script
    CONFIG += qtestlib
}

!unix|mac: LIBS += -L$$PWD/../../bin -lQupZilla
!mac:unix: LIBS += $$PWD/../../bin/libQupZilla.so

QMAKE_LFLAGS+=$${QMAKE_LFLAGS_RPATH}$$PWD/../../bin

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
               $$PWD/../../src/lib/rss \
               $$PWD/../../src/lib/session \
               $$PWD/../../src/lib/sidebar \
               $$PWD/../../src/lib/tabwidget \
               $$PWD/../../src/lib/tools \
               $$PWD/../../src/lib/webkit \
               $$PWD/../../src/lib/webtab \
