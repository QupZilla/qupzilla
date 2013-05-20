include(../../defines.pri)

TARGET = $$qtLibraryTarget(KWalletPasswords)

SOURCES += kwalletplugin.cpp \
    kwalletpasswordbackend.cpp

HEADERS += kwalletplugin.h \
    kwalletpasswordbackend.h

RESOURCES += kwalletpasswords.qrc

TRANSLATIONS = translations/cs_CZ.ts \
               translations/sr_BA.ts \
               translations/sr_BA@latin.ts \
               translations/sr_RS.ts \
               translations/sr_RS@latin.ts

LIBS += -lkdeui

include(../../plugins.pri)
