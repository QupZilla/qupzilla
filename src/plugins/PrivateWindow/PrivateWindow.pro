#-------------------------------------------------
#
# Project created by QtCreator 2013-02-20T15:23:18
#
#-------------------------------------------------
TARGET = $$qtLibraryTarget(PrivateWindow)
# OS/2 allows only 8 chars in TARGET
os2: TARGET = PrivWinP

SOURCES += privatewindowplugin.cpp
HEADERS += privatewindowplugin.h
RESOURCES += privatewindow.qrc

TRANSLATIONS += translations/de_DE.ts\

include(../../plugins.pri)
