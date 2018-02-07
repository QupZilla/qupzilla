TARGET = $$qtLibraryTarget(VerticalTabs)

SOURCES += verticaltabsplugin.cpp \
           verticaltabscontroller.cpp \
           verticaltabswidget.cpp \
           verticaltabssettings.cpp \
           tabtreeview.cpp \
           tabtreedelegate.cpp \
           loadinganimator.cpp \
           tabfiltermodel.cpp \
           tablistview.cpp \
           tablistdelegate.cpp \
           verticaltabsschemehandler.cpp \

HEADERS += verticaltabsplugin.h \
           verticaltabscontroller.h \
           verticaltabswidget.h \
           verticaltabssettings.h \
           tabtreeview.h \
           tabtreedelegate.h \
           loadinganimator.h \
           tabfiltermodel.h \
           tablistview.h \
           tablistdelegate.h \
           verticaltabsschemehandler.h \

RESOURCES += verticaltabs.qrc

FORMS += verticaltabssettings.ui

PLUGIN_DIR = $$PWD
include(../../plugins.pri)
