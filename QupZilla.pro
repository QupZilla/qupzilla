#-------------------------------------------------
#
#           QupZilla - QtWebKit browser
#
# Project created by QtCreator 2010-12-18T14:53:41
#
#-------------------------------------------------

lessThan(QT_VERSION, 4.7) {
    error("QupZilla requires at least Qt 4.7!")
}

# Create plugins directory first on Mac / Linux
mac|unix: system(test -d bin/plugins || mkdir bin/plugins)

TEMPLATE = subdirs
SUBDIRS = src/lib src/main src/plugins
CONFIG += ordered
