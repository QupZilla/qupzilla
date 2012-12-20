include(defines.pri)

INCLUDEPATH += $$PWD/lib/3rdparty\
               $$PWD/lib/app\
               $$PWD/lib/autofill\
               $$PWD/lib/bookmarks\
               $$PWD/lib/cookies\
               $$PWD/lib/session\
               $$PWD/lib/downloads\
               $$PWD/lib/history\
               $$PWD/lib/navigation\
               $$PWD/lib/network\
               $$PWD/lib/other\
               $$PWD/lib/preferences\
               $$PWD/lib/rss\
               $$PWD/lib/tools\
               $$PWD/lib/utils\
               $$PWD/lib/webview\
               $$PWD/lib/plugins\
               $$PWD/lib/sidebar\
               $$PWD/lib/data\
               $$PWD/lib/adblock\
               $$PWD/lib/desktopnotifications\
               $$PWD/lib/opensearch\
               $$PWD/lib/bookmarksimport\
               $$PWD/lib/popupwindow\

TEMPLATE = lib
CONFIG += plugin
DESTDIR = $$PWD/../bin/plugins/

isEqual(QT_MAJOR_VERSION, 5) {
    QT *= webkitwidgets network
} else {
    QT *= webkit network
}

OBJECTS_DIR = build
MOC_DIR = build
RCC_DIR = build
UI_DIR = build

!unix|mac: LIBS += -L$$PWD/../bin -lQupZilla
!mac:unix: LIBS += $$PWD/../bin/libQupZilla.so

!mac:unix {
    target.path = $$library_folder/qupzilla

    INSTALLS += target
}

updateqm.input = TRANSLATIONS
updateqm.output = locale/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$QMAKE_LRELEASE -silent ${QMAKE_FILE_IN} -qm locale/${QMAKE_FILE_BASE}.qm
updateqm.CONFIG += no_link target_predeps
QMAKE_EXTRA_COMPILERS += updateqm
