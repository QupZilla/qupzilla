include(defines.pri)

INCLUDEPATH += $$PWD/lib/3rdparty \
               $$PWD/lib/adblock \
               $$PWD/lib/app \
               $$PWD/lib/autofill \
               $$PWD/lib/bookmarks \
               $$PWD/lib/cookies \
               $$PWD/lib/downloads \
               $$PWD/lib/history \
               $$PWD/lib/navigation \
               $$PWD/lib/network \
               $$PWD/lib/notifications \
               $$PWD/lib/opensearch \
               $$PWD/lib/other \
               $$PWD/lib/plugins \
               $$PWD/lib/popupwindow \
               $$PWD/lib/preferences \
               $$PWD/lib/rss \
               $$PWD/lib/session \
               $$PWD/lib/sidebar \
               $$PWD/lib/tabwidget \
               $$PWD/lib/tools \
               $$PWD/lib/webkit \
               $$PWD/lib/webtab \

DEPENDPATH += $$INCLUDEPATH \
              $$PWD/lib/data \

TEMPLATE = lib
CONFIG += plugin
DESTDIR = $$QZ_DESTDIR/plugins/

isEqual(QT_MAJOR_VERSION, 5) {
    QT *= webkitwidgets network
} else {
    QT *= webkit network
}

OBJECTS_DIR = build
MOC_DIR = build
RCC_DIR = build
UI_DIR = build

!unix|mac: LIBS += -L$$QZ_DESTDIR -lQupZilla
!mac:unix: LIBS += $$QZ_DESTDIR/libQupZilla.so

!mac:unix {
    target.path = $$library_folder/qupzilla

    INSTALLS += target
}

updateqm.input = TRANSLATIONS
updateqm.output = $$PLUGIN_DIR/locale/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$QMAKE_LRELEASE -silent ${QMAKE_FILE_IN} -qm $$PLUGIN_DIR/locale/${QMAKE_FILE_BASE}.qm
updateqm.CONFIG += no_link target_predeps
QMAKE_EXTRA_COMPILERS += updateqm
