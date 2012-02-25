TEMPLATE = subdirs
SUBDIRS  = MouseGestures

# TestPlugin only in debug build
CONFIG(debug, debug|release): SUBDIRS += TestPlugin
