TARGET = $$qtLibraryTarget(ImageFinder)
# OS/2 allows only 8 chars in TARGET
os2: TARGET = ImgFndr

HEADERS += \
    imagefinderplugin.h \
    imagefinder.h \
    imagefindersettings.h \

SOURCES += \
    imagefinderplugin.cpp \
    imagefinder.cpp \
    imagefindersettings.cpp \

RESOURCES += imagefinder.qrc

FORMS += imagefindersettings.ui

PLUGIN_DIR = $$PWD
include(../../plugins.pri)
