include(../../defines.pri)

TARGET = $$qtLibraryTarget(StatusBarIcons)

SOURCES += statusbariconsplugin.cpp \
    sbi_iconsmanager.cpp \
    sbi_imagesicon.cpp \
    sbi_javascripticon.cpp \
    sbi_networkicon.cpp \
    sbi_networkproxy.cpp

HEADERS += statusbariconsplugin.h \
    sbi_iconsmanager.h \
    sbi_imagesicon.h \
    sbi_javascripticon.h \
    sbi_networkicon.h \
    sbi_networkproxy.h

RESOURCES += statusbaricons.qrc

include(../../plugins.pri)
