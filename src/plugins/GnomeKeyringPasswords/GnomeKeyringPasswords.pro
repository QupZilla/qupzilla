include(../../defines.pri)

TARGET = $$qtLibraryTarget(GnomeKeyringPasswords)

SOURCES += gnomekeyringplugin.cpp \
    gnomekeyringpasswordbackend.cpp

HEADERS += gnomekeyringplugin.h \
    gnomekeyringpasswordbackend.h

RESOURCES += gnomekeyringpasswords.qrc

TRANSLATIONS = translations/cs_CZ.ts \
               translations/de_DE.ts \
               translations/el_GR.ts \
               translations/fr_FR.ts \
               translations/he_IL.ts \
               translations/ja_JP.ts \
               translations/nl_NL.ts \
               translations/pl_PL.ts \
               translations/pt_PT.ts \
               translations/ru_RU.ts \
               translations/sr_BA.ts \
               translations/sr_BA@latin.ts \
               translations/sr_RS.ts \
               translations/sr_RS@latin.ts \
               translations/zh_CN.ts \
               translations/zh_TW.ts

LIBS += $$system(pkg-config --libs gnome-keyring-1)
QMAKE_CXXFLAGS += $$system(pkg-config --cflags gnome-keyring-1)

include(../../plugins.pri)
