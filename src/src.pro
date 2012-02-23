TEMPLATE = subdirs
SUBDIRS = lib main
build_plugins: SUBDIRS += plugins

CONFIG += ordered
