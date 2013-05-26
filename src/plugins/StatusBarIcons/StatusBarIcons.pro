include(../../defines.pri)

TARGET = $$qtLibraryTarget(StatusBarIcons)

SOURCES += statusbariconsplugin.cpp \
    sbi_iconsmanager.cpp \
    sbi_imagesicon.cpp

HEADERS += statusbariconsplugin.h \
    sbi_iconsmanager.h \
    sbi_imagesicon.h

RESOURCES += statusbaricons.qrc

include(../../plugins.pri)
