TEMPLATE = subdirs
SUBDIRS  = MouseGestures AccessKeysNavigation

# TestPlugin only in debug build
CONFIG(debug, debug|release): SUBDIRS += TestPlugin
