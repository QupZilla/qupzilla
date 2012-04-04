include(defines.pri)

INCLUDEPATH += $$PWD/lib/3rdparty\
               $$PWD/lib/app\
               $$PWD/lib/autofill\
               $$PWD/lib/bookmarks\
               $$PWD/lib/cookies\
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
QT *= webkit network
DESTDIR = $$PWD/../bin/plugins/

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

mac {
    target.path = Contents/MacOS/qupzilla
    QMAKE_BUNDLE_DATA += target
}

updateqm.input = TRANSLATIONS
updateqm.output = locale/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$QMAKE_LRELEASE -silent ${QMAKE_FILE_IN} -qm locale/${QMAKE_FILE_BASE}.qm
updateqm.CONFIG += no_link target_predeps
QMAKE_EXTRA_COMPILERS += updateqm
