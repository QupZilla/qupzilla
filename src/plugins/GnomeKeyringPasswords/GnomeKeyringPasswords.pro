include(../../defines.pri)

TARGET = $$qtLibraryTarget(GnomeKeyringPasswords)

SOURCES += gnomekeyringplugin.cpp \
    gnomekeyringpasswordbackend.cpp

HEADERS += gnomekeyringplugin.h \
    gnomekeyringpasswordbackend.h

RESOURCES += gnomekeyringpasswords.qrc

TRANSLATIONS = translations/cs_CZ.ts \
               translations/sr_BA.ts \
               translations/sr_BA@latin.ts \
               translations/sr_RS.ts \
               translations/sr_RS@latin.ts

LIBS += $$system(pkg-config --libs gnome-keyring-1)
QMAKE_CXXFLAGS += $$system(pkg-config --cflags gnome-keyring-1)

include(../../plugins.pri)
