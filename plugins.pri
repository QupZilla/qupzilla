include(defines.pri)

INCLUDEPATH += $$PWD/src/3rdparty\
               $$PWD/src/app\
               $$PWD/src/autofill\
               $$PWD/src/bookmarks\
               $$PWD/src/cookies\
               $$PWD/src/downloads\
               $$PWD/src/history\
               $$PWD/src/navigation\
               $$PWD/src/network\
               $$PWD/src/other\
               $$PWD/src/preferences\
               $$PWD/src/rss\
               $$PWD/src/tools\
               $$PWD/src/utils\
               $$PWD/src/webview\
               $$PWD/src/plugins\
               $$PWD/src/sidebar\
               $$PWD/src/data\
               $$PWD/src/adblock\
               $$PWD/src/desktopnotifications\
               $$PWD/src/opensearch\
               $$PWD/src/bookmarksimport\
               $$PWD/src/popupwindow\

TEMPLATE = lib
CONFIG += plugin
DESTDIR = $$PWD/bin/plugins/

OBJECTS_DIR = build
MOC_DIR = build
RCC_DIR = build
UI_DIR = build

LIBS += -L $$PWD/bin -lqupzilla

!mac:unix {
    target.path = $$library_folder/qupzilla

    INSTALLS += target
}
