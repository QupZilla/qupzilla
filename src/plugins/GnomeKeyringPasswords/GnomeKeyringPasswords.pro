include(../../defines.pri)

!mac:unix:contains(DEFINES, "GNOME_INTEGRATION"):system(pkg-config --exists gnome-keyring-1) {
    TARGET = $$qtLibraryTarget(GnomeKeyringPasswords)

    SOURCES += gnomekeyringplugin.cpp \
        gnomekeyringpasswordbackend.cpp

    HEADERS += gnomekeyringplugin.h \
        gnomekeyringpasswordbackend.h

    RESOURCES += gnomekeyringpasswords.qrc

    TRANSLATIONS = translations/cs_CZ.ts \

    LIBS += $$system(pkg-config --libs gnome-keyring-1)
    QMAKE_CXXFLAGS += $$system(pkg-config --cflags gnome-keyring-1)
}

include(../../plugins.pri)
