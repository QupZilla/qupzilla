lessThan(QT_VERSION, 4.7) {
    error("QupZilla requires at least Qt 4.7!")
}

TEMPLATE = subdirs

build_plugins {
    SUBDIRS + = plugins
}

SUBDIRS += src
