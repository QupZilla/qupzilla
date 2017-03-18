#-------------------------------------------------
#
#           QupZilla - QtWebKit browser
#
# Project created by QtCreator 2010-12-18T14:53:41
#
#-------------------------------------------------

lessThan(QT_VERSION, 5.8) {
    error("QupZilla requires at least Qt 5.8!")
}

lessThan(QT.webengine.VERSION, 5.8) {
    error("QupZilla requires at least QtWebEngine 5.8!")
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
