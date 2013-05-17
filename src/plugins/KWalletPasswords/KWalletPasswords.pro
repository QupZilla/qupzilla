include(../../defines.pri)

contains(DEFINES, "KDE") {
    TARGET = $$qtLibraryTarget(KWalletPasswords)

    SOURCES += kwalletplugin.cpp \
        kwalletpasswordbackend.cpp

    HEADERS += kwalletplugin.h \
        kwalletpasswordbackend.h

    RESOURCES += kwalletpasswords.qrc

    TRANSLATIONS = translations/cs_CZ.ts \

    LIBS += -lkdeui
}

include(../../plugins.pri)
