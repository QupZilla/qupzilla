#-------------------------------------------------
#
#           QupZilla - Qt web browser
#
# Project created by QtCreator 2010-12-18T14:53:41
#
#-------------------------------------------------

QT_MAJOR_VERSION = $$section(QT_VERSION, ., 0, 0)
QT_MINOR_VERSION = $$section(QT_VERSION, ., 1, 1)
QT_PATCH_VERSION = $$section(QT_VERSION, ., 2, 2)

lessThan(QT_MAJOR_VERSION, 5)|lessThan(QT_MINOR_VERSION, 9)|lessThan(QT_PATCH_VERSION, 0) {
    error("QupZilla requires at least Qt 5.9.0!")
}

# Create plugins directory first on Mac / Linux
mac|unix: system(test -d bin/plugins || mkdir bin/plugins)

TEMPLATE = subdirs

src_lib.subdir = src/lib
src_lib.target = sub-src-lib

src_main.subdir = src/main
src_main.depends = sub-src-lib

src_plugins.subdir = src/plugins
src_plugins.depends = sub-src-lib

SUBDIRS += src_lib src_main src_plugins

mac: {
    macdeploysh.target = bundle
    macdeploysh.commands = mac/macdeploy.sh $$[QT_INSTALL_BINS]/macdeployqt
    QMAKE_EXTRA_TARGETS += macdeploysh
}
