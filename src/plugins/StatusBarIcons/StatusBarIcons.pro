include(../../defines.pri)

TARGET = $$qtLibraryTarget(StatusBarIcons)

SOURCES += statusbariconsplugin.cpp \
    sbi_iconsmanager.cpp \
    sbi_imagesicon.cpp \
    sbi_javascripticon.cpp \
    sbi_networkicon.cpp \
    sbi_networkproxy.cpp \
    sbi_proxywidget.cpp \
    sbi_networkicondialog.cpp \
    sbi_networkmanager.cpp \
    sbi_settingsdialog.cpp

HEADERS += statusbariconsplugin.h \
    sbi_iconsmanager.h \
    sbi_imagesicon.h \
    sbi_javascripticon.h \
    sbi_networkicon.h \
    sbi_networkproxy.h \
    sbi_proxywidget.h \
    sbi_networkicondialog.h \
    sbi_networkmanager.h \
    sbi_settingsdialog.h

RESOURCES += statusbaricons.qrc

include(../../plugins.pri)

FORMS += \
    sbi_proxywidget.ui \
    sbi_networkicondialog.ui \
    sbi_settingsdialog.ui
